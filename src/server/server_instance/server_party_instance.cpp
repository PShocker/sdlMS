#include "src/server/server_instance/server_party_instance.h"

#include "src/client/game/game_party.h"
#include "src/client/game/game_popup_tip.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/party_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/system/ui/party_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/request/client_request.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_client_instance.h"

#include <algorithm>
#include <memory>
#include <optional>

namespace sdlms {

namespace {

// 095：邀请 30 秒内有效。
constexpr uint64_t kInviteTimeoutMs = 30 * 1000;

std::vector<ServerPartyInvite>& Invites() {
  static std::vector<ServerPartyInvite> invites;
  return invites;
}

uint64_t NextPartyId() {
  static uint64_t next = 1;
  return next++;
}

bool ClientAlive(uint64_t client_id) {
  return client_id != 0 &&
         ServerClientInstance::clients_.contains(client_id);
}

// 从 player states 里取 HP / MAX_HP（客户端上报、服务端实时维护）。
uint32_t LoadStateVal(const fbs::PlayerT& p, fbs::StateEnum type) {
  if (p.character == nullptr) {
    return 0;
  }
  for (const auto& st : p.character->states) {
    if (st->state == type) {
      return static_cast<uint32_t>(std::max<int64_t>(0, st->val));
    }
  }
  return 0;
}

// 给队员列表包填 players + hp/max_hp（对齐 095 PARTYDATA 全量）。
void FillMembers(fbs::ServerCharacterPartyT& m, const ServerParty& party) {
  for (auto c_id : party.client_ids) {
    if (!ClientAlive(c_id)) {
      continue;
    }
    const auto& client = ServerClientInstance::clients_.at(c_id);
    m.players.push_back(std::make_unique<fbs::PlayerT>(client.player_t));
    m.hp.push_back(LoadStateVal(client.player_t, fbs::StateEnum_HP));
    m.max_hp.push_back(
        LoadStateVal(client.player_t, fbs::StateEnum_MAX_HP));
  }
}

void SendToClient(uint64_t client_id, fbs::ServerCharacterPartyT& m) {
  if (ClientAlive(client_id)) {
    ServerResponse::SendToClient(client_id, m);
  }
}

// 广播全量队伍（成员变动后所有人刷新，095 step 7）。
void BroadcastParty(const ServerParty& party, uint8_t skip_step = 7) {
  fbs::ServerCharacterPartyT m;
  m.step = skip_step;
  m.to_id = party.captain;
  FillMembers(m, party);
  for (auto c_id : party.client_ids) {
    SendToClient(c_id, m);
  }
}

// 找到 client 所在队伍的迭代器。
std::flat_map<uint64_t, ServerParty>::iterator FindClientParty(
    uint64_t client_id) {
  for (auto it = ServerPartyInstance::data_.begin();
       it != ServerPartyInstance::data_.end(); ++it) {
    if (std::ranges::contains(it->second.client_ids, client_id)) {
      return it;
    }
  }
  return ServerPartyInstance::data_.end();
}

// 队伍里是否所有成员除例外者都不在线 → 解散。
void PruneParty(std::flat_map<uint64_t, ServerParty>::iterator it) {
  auto& party = it->second;
  // 掉线成员直接移除（095 成员下线即离队）。
  std::erase_if(party.client_ids, [](uint64_t c_id) {
    return !ClientAlive(c_id);
  });
  if (party.client_ids.empty()) {
    ServerPartyInstance::data_.erase(it);
    return;
  }
  // 队长掉线：转让给首位在线成员（095 step 31 换队长）。
  if (!std::ranges::contains(party.client_ids, party.captain)) {
    party.captain = party.client_ids.front();
  }
  BroadcastParty(party);
}

// 成员离队通用：leave_client 离队并广播 12（x: 0=主动离队 1=被踢）。
// 返回 true 表示房间已删除。
bool LeaveParty(std::flat_map<uint64_t, ServerParty>::iterator it,
                uint64_t leave_client, bool kicked) {
  auto& party = it->second;
  const bool is_captain = party.captain == leave_client;

  // 先广播变动（095 step 12：对方收到 "%s 离开/被退出"，本人收提示）。
  auto& clients = ServerClientInstance::clients_;
  std::u16string leaver_name;
  if (clients.contains(leave_client) &&
      clients.at(leave_client).player_t.character != nullptr) {
    leaver_name = {clients.at(leave_client)
                       .player_t.character->name.begin(),
                   clients.at(leave_client)
                       .player_t.character->name.end()};
  }
  for (auto c_id : party.client_ids) {
    fbs::ServerCharacterPartyT m;
    m.step = 12;
    m.x = kicked ? 1 : 0;
    m.to_id = leave_client;
    if (clients.contains(leave_client)) {
      m.player = std::make_unique<fbs::PlayerT>(
          clients.at(leave_client).player_t);
    }
    SendToClient(c_id, m);
  }

  std::erase(party.client_ids, leave_client);
  if (party.client_ids.empty() || is_captain) {
    // 队长离开 / 全员走光：解散（095 step 12 flag=0 分支）。
    ServerPartyInstance::data_.erase(it);
    return true;
  }
  BroadcastParty(party);
  return false;
}

}  // namespace

ServerParty& ServerPartyInstance::LoadParty(uint64_t captain_id) {
  if (data_.contains(captain_id)) {
    return data_.at(captain_id);
  }
  data_[captain_id] = {.captain = captain_id};
  return data_[captain_id];
}

void ServerPartyInstance::HandleParty(uint64_t client_id,
                                      fbs::ClientCharacterPartyT& r) {
  if (!ClientAlive(client_id)) {
    return;
  }
  auto& clients = ServerClientInstance::clients_;

  switch (r.step) {
    // 邀请入队（095 CP 145 step4）：右键玩家 Party。
    case 0: {
      const uint64_t to_id = r.to_id;
      if (to_id == client_id || !ClientAlive(to_id)) {
        return;
      }
      // 邀请人必须无队或为队长（095 SendJoinPartyMsg：非队长不可邀请）。
      auto it = FindClientParty(client_id);
      if (it != data_.end()) {
        if (it->second.captain != client_id) {
          // 17：不是队长。
          fbs::ServerCharacterPartyT m;
          m.step = 17;
          SendToClient(client_id, m);
          return;
        }
        // 18：队伍已满。
        if (it->second.client_ids.size() >= ServerParty::kMaxMembers) {
          fbs::ServerCharacterPartyT m;
          m.step = 18;
          SendToClient(client_id, m);
          return;
        }
      }
      // 目标已在队（095 step 9：已经加入其他组）。
      if (FindClientParty(to_id) != data_.end()) {
        fbs::ServerCharacterPartyT m;
        m.step = 17;
        SendToClient(client_id, m);
        return;
      }
      // 同一目标 30 秒内不重复邀请。
      auto& invites = Invites();
      if (std::ranges::any_of(invites, [&](const ServerPartyInvite& inv) {
            return inv.to_client == to_id && inv.from_captain == client_id &&
                   inv.expire > Window::dt_now_;
          })) {
        return;
      }
      ServerPartyInvite inv;
      inv.from_captain = client_id;
      inv.to_client = to_id;
      inv.expire = Window::dt_now_ + kInviteTimeoutMs;
      invites.push_back(inv);

      // 给被邀请人弹确认框（095 step 4 → CUIFadeYesNo CreatePartyInvite）。
      fbs::ServerCharacterPartyT m;
      m.step = 5;
      m.to_id = client_id;
      m.player =
          std::make_unique<fbs::PlayerT>(clients.at(client_id).player_t);
      SendToClient(to_id, m);
      // 给邀请人回执（095 step 22："你已邀请%s"，带被邀请人 Player）。
      fbs::ServerCharacterPartyT ack;
      ack.step = 16;
      ack.to_id = to_id;
      ack.player = std::make_unique<fbs::PlayerT>(clients.at(to_id).player_t);
      SendToClient(client_id, ack);
      return;
    }
    // 接受邀请（095 CP 145 step3）：to_id = 邀请人。
    case 1: {
      const uint64_t inviter = r.to_id;
      auto& invites = Invites();
      // 必须存在有效邀请。
      auto inv_it = std::ranges::find_if(
          invites, [&](const ServerPartyInvite& inv) {
            return inv.to_client == client_id &&
                   inv.from_captain == inviter && inv.expire > Window::dt_now_;
          });
      if (inv_it == invites.end() || !ClientAlive(inviter)) {
        return;
      }
      invites.erase(inv_it);
      // 邀请人没队时自动替他建队（095：邀请即建队，Boss=邀请人）。
      auto inviter_party = FindClientParty(inviter);
      if (inviter_party == data_.end()) {
        if (FindClientParty(client_id) != data_.end()) {
          return;  // 接受者已在其它队
        }
        ServerParty party;
        party.id = NextPartyId();
        party.captain = inviter;
        party.client_ids.push_back(inviter);
        data_[inviter] = party;
        inviter_party = data_.find(inviter);
      }
      // 邀请人已不是队长（被转让等）：按无效邀请处理。
      if (inviter_party->second.captain != inviter) {
        return;
      }
      if (FindClientParty(client_id) != data_.end()) {
        return;
      }
      if (inviter_party->second.client_ids.size() >=
          ServerParty::kMaxMembers) {
        fbs::ServerCharacterPartyT m;
        m.step = 18;
        SendToClient(inviter, m);
        return;
      }
      inviter_party->second.client_ids.push_back(client_id);
      // 全员广播新成员（095 step 15）；收到方 UI 以 step7 全量刷新。
      {
        fbs::ServerCharacterPartyT m;
        m.step = 15;
        if (clients.contains(client_id)) {
          m.player = std::make_unique<fbs::PlayerT>(
              clients.at(client_id).player_t);
        }
        for (auto c_id : inviter_party->second.client_ids) {
          SendToClient(c_id, m);
        }
      }
      BroadcastParty(inviter_party->second);
      return;
    }
    // 拒绝邀请：通知邀请人（095 无包、本地忽略；这里给 19 提升体验）。
    case 2: {
      auto& invites = Invites();
      std::erase_if(invites, [&](const ServerPartyInvite& inv) {
        return inv.to_client == client_id &&
               inv.from_captain == r.to_id;
      });
      fbs::ServerCharacterPartyT m;
      m.step = 19;
      m.to_id = client_id;
      if (clients.contains(client_id)) {
        m.player =
            std::make_unique<fbs::PlayerT>(clients.at(client_id).player_t);
      }
      SendToClient(r.to_id, m);
      return;
    }
    // 退队（095 CP 145 step2）。
    case 3: {
      auto it = FindClientParty(client_id);
      if (it == data_.end()) {
        // 13：没有参加的组队。
        fbs::ServerCharacterPartyT m;
        m.step = 13;
        SendToClient(client_id, m);
        return;
      }
      LeaveParty(it, client_id, false);
      return;
    }
    // 踢人（095 CP 145 step5）：仅队长。
    case 4: {
      auto it = FindClientParty(client_id);
      if (it == data_.end() || it->second.captain != client_id) {
        fbs::ServerCharacterPartyT m;
        m.step = 17;
        SendToClient(client_id, m);
        return;
      }
      if (!std::ranges::contains(it->second.client_ids, r.to_id) ||
          r.to_id == client_id) {
        return;
      }
      LeaveParty(it, r.to_id, true);
      return;
    }
    // 转让队长（095 CP 145 step6）：仅队长，必须是队员。
    case 5: {
      auto it = FindClientParty(client_id);
      if (it == data_.end() || it->second.captain != client_id) {
        fbs::ServerCharacterPartyT m;
        m.step = 17;
        SendToClient(client_id, m);
        return;
      }
      if (!std::ranges::contains(it->second.client_ids, r.to_id) ||
          r.to_id == client_id) {
        return;
      }
      it->second.captain = r.to_id;
      // 队长移到首位（成员列表以队长开头）。
      std::erase(it->second.client_ids, r.to_id);
      it->second.client_ids.insert(it->second.client_ids.begin(), r.to_id);
      BroadcastParty(it->second);
      return;
    }
    default: {
      return;
    }
  }
}

bool ServerPartyInstance::Run() {
  // 清理过期邀请。
  auto& invites = Invites();
  std::erase_if(invites, [](const ServerPartyInvite& inv) {
    return inv.expire <= Window::dt_now_;
  });
  // 清理掉线成员。
  for (auto it = data_.begin(); it != data_.end();) {
    bool any_offline = std::ranges::any_of(it->second.client_ids,
                                           [](uint64_t c_id) {
                                             return !ClientAlive(c_id);
                                           });
    if (any_offline) {
      PruneParty(it);
      it = data_.begin();
      continue;
    }
    ++it;
  }
  return true;
}

void ServerPartyInstance::SyncClientParty(uint64_t client_id) {
  auto it = FindClientParty(client_id);
  if (it == data_.end()) {
    return;
  }
  BroadcastParty(it->second);
}

void ServerPartyInstance::HandleServerParty(uint64_t client_id,
                                            fbs::ServerCharacterPartyT& r) {
  auto& data = PartyGameInstance::data_;
  auto* chat = &PartyUiSystem::chat_log_;
  auto add_chat = [&](const std::u16string& line) {
    chat->push_back(line);
    if (chat->size() > 100) {
      chat->erase(chat->begin());
    }
  };
  auto name_of = [](const fbs::PlayerT* p) -> std::u16string {
    if (p != nullptr && p->character != nullptr) {
      return {p->character->name.begin(), p->character->name.end()};
    }
    return u"???";
  };

  switch (r.step) {
    // 邀请弹窗（095 CUIFadeYesNo CreatePartyInvite，player=邀请人）。
    case 5: {
      if (r.player == nullptr) {
        return;
      }
      PopupTipGameInstance::AddTip(PopupTipEnums::kParty, *r.player);
      return;
    }
    // 全量队伍刷新。
    case 7: {
      data = GameParty{};
      auto& pt = data.emplace();
      pt.captain_id = r.to_id;
      for (size_t i = 0; i < r.players.size(); ++i) {
        GamePartyMember m;
        m.player = *r.players[i];
        if (i < r.hp.size()) {
          m.hp = r.hp[i];
        }
        if (i < r.max_hp.size()) {
          m.max_hp = r.max_hp[i];
        }
        pt.members.push_back(std::move(m));
      }
      PartyUiSystem::Open();
      return;
    }
    // 成员变动：x 1=自己被踢 0=离队/解散；player=变动者。
    case 12: {
      const auto leaver = name_of(r.player.get());
      const bool me = r.to_id == ServerMain::local_addr_;
      if (me) {
        if (r.x == 1) {
          add_chat(u"从组队被退出了。");  // 095 String[324]
        } else if (!data.has_value()) {
          add_chat(u"队长退出，组队结束了。");  // 095 String[328]
        } else {
          add_chat(u"组队结束。");  // 095 String[325]
        }
        data = std::nullopt;
        PartyUiSystem::Close();
      } else {
        if (r.x == 1) {
          // 095 String[326]：'%s'玩家从组队被退出了。
          add_chat(leaver + u"玩家从组队被退出了。");
        } else {
          // 095 String[327]：'%s'玩家离开组队。
          add_chat(leaver + u"玩家离开组队。");
        }
      }
      return;
    }
    // 新成员加入（player=加入者；随后 step7 全量会刷新数据）。
    case 15: {
      const auto joiner = name_of(r.player.get());
      if (joiner == CharacterGameInstance::self_.name) {
        add_chat(u"加入组队。");  // 095 String[330]
      } else {
        // 095 String[331]：'%s'玩家参加到组队。
        add_chat(joiner + u"玩家参加到组队。");
      }
      return;
    }
    // 已邀请回执（095 String[5514]：你已邀请"%s"加入你的队伍。）。
    case 16: {
      const auto invited = name_of(r.player.get());
      if (invited != u"???") {
        add_chat(u"你已邀请" + invited + u"加入你的队伍。");
      } else {
        add_chat(u"已发送组队邀请。");
      }
      return;
    }
    // 17：已经在其他组 / 不是队长。
    case 17: {
      add_chat(u"已经加入其他组。");  // 095 String[332]
      return;
    }
    // 18：组队成员已满。
    case 18: {
      add_chat(u"组成员已满。");  // 095 String[335]
      return;
    }
    // 19：对方拒绝了邀请。
    case 19: {
      const auto refuser = name_of(r.player.get());
      if (refuser != u"???") {
        add_chat(refuser + u"拒绝了组队邀请。");
      } else {
        add_chat(u"对方拒绝了组队邀请。");
      }
      return;
    }
    // 13：没有参加的组队。
    case 13: {
      add_chat(u"没有参加的组队。");  // 095 String[334]
      return;
    }
    default: {
      return;
    }
  }
}

std::optional<ServerParty> ServerPartyInstance::LoadClientParty(
    uint64_t client_id) {
  for (const auto& [k, v] : data_) {
    auto clients = v.client_ids;
    if (std::ranges::contains(clients, client_id)) {
      return v;
    }
  }
  return std::nullopt;
}

}  // namespace sdlms