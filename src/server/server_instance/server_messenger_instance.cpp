#include "src/server/server_instance/server_messenger_instance.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/system/ui/maple_chat_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/request/client_request.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_messenger.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_main.h"

#include <algorithm>
#include <memory>
#include <optional>

namespace sdlms {

namespace {

// 095：邀请 30 秒内有效。
constexpr uint64_t kInviteTimeoutMs = 30 * 1000;

std::flat_map<uint64_t, ServerMessengerRoom>& Rooms() {
  static std::flat_map<uint64_t, ServerMessengerRoom> rooms;
  return rooms;
}

std::vector<ServerMessengerInvite>& Invites() {
  static std::vector<ServerMessengerInvite> invites;
  return invites;
}

uint64_t NextRoomId() {
  static uint64_t next = 1;
  return next++;
}

bool ClientAlive(uint64_t client_id) {
  return client_id != 0 &&
         ServerClientInstance::clients_.contains(client_id);
}

std::u16string LoadName(uint64_t client_id) {
  auto& clients = ServerClientInstance::clients_;
  if (clients.contains(client_id) &&
      clients.at(client_id).player_t.character != nullptr) {
    return {clients.at(client_id).player_t.character->name.begin(),
            clients.at(client_id).player_t.character->name.end()};
  }
  return {};
}

// 按名字找在线玩家（095 SendInviteMsg 用名字邀请）。
uint64_t FindClientByName(const std::u16string& name) {
  if (name.empty()) {
    return 0;
  }
  for (const auto& [id, client] : ServerClientInstance::clients_) {
    if (client.player_t.character != nullptr) {
      std::u16string n{client.player_t.character->name.begin(),
                       client.player_t.character->name.end()};
      if (n == name) {
        return id;
      }
    }
  }
  return 0;
}

std::unique_ptr<fbs::PlayerT> LoadPlayer(uint64_t client_id) {
  if (!ClientAlive(client_id)) {
    return nullptr;
  }
  return std::make_unique<fbs::PlayerT>(
      ServerClientInstance::clients_.at(client_id).player_t);
}

void SendToClient(uint64_t client_id, fbs::ServerCharacterMessengerT& m) {
  if (ClientAlive(client_id)) {
    ServerResponse::SendToClient(client_id, m);
  }
}

// 成员快照：players 下标 = slot（离线成员保留 nullptr 占位以对齐下标）。
void FillSnapshot(fbs::ServerCharacterMessengerT& m,
                  const ServerMessengerRoom& room) {
  for (size_t slot = 0; slot < room.client_ids.size(); ++slot) {
    m.players.push_back(LoadPlayer(room.client_ids[slot]));
  }
}

std::flat_map<uint64_t, ServerMessengerRoom>::iterator FindRoomBySn(
    uint64_t sn) {
  return Rooms().find(sn);
}

std::flat_map<uint64_t, ServerMessengerRoom>::iterator FindClientRoom(
    uint64_t client_id) {
  for (auto it = Rooms().begin(); it != Rooms().end(); ++it) {
    if (std::ranges::contains(it->second.client_ids, client_id)) {
      return it;
    }
  }
  return Rooms().end();
}

int FindSlot(const ServerMessengerRoom& room, uint64_t client_id) {
  auto it = std::ranges::find(room.client_ids, client_id);
  if (it == room.client_ids.end()) {
    return -1;
  }
  return static_cast<int>(it - room.client_ids.begin());
}

// 离开房间：广播 step2（095 OnLeave），空房删除。返回 true 表示房间已删。
bool LeaveRoom(std::flat_map<uint64_t, ServerMessengerRoom>::iterator it,
               uint64_t client_id) {
  auto& room = it->second;
  const int slot = FindSlot(room, client_id);
  if (slot < 0) {
    return false;
  }
  auto player = LoadPlayer(client_id);
  for (auto c_id : room.client_ids) {
    if (c_id == client_id || !ClientAlive(c_id)) {
      continue;
    }
    fbs::ServerCharacterMessengerT m;
    m.step = 2;
    m.to_id = client_id;
    m.x = static_cast<uint8_t>(slot);
    if (player != nullptr) {
      m.player = std::make_unique<fbs::PlayerT>(*player);
    }
    SendToClient(c_id, m);
  }
  room.client_ids.erase(room.client_ids.begin() + slot);
  if (room.client_ids.empty()) {
    Rooms().erase(it);
    return true;
  }
  return false;
}

// 掉线清理：成员不在服务端名单时按离房处理。返回 true 需要重扫。
bool PruneRoom(std::flat_map<uint64_t, ServerMessengerRoom>::iterator it) {
  auto& room = it->second;
  for (size_t i = 0; i < room.client_ids.size();) {
    if (!ClientAlive(room.client_ids[i])) {
      if (LeaveRoom(it, room.client_ids[i])) {
        return true;  // 房间已删除，从头重扫
      }
      continue;  // LeaveRoom 已移除该成员，i 不递增
    }
    ++i;
  }
  return false;
}

}  // namespace

void ServerMessengerInstance::HandleMessenger(
    uint64_t client_id, fbs::ClientCharacterMessengerT& r) {
  if (!ClientAlive(client_id)) {
    return;
  }

  switch (r.step) {
    // 进入/创建房间（095 OnCreate：CP 143 step0 + SN）。
    case 0: {
      const uint64_t sn = r.to_id;
      // 已在其他房间：先离再进（095 单例窗口由客户端保证，服务端兜底）。
      auto old = FindClientRoom(client_id);
      if (old != Rooms().end()) {
        if (old->first == sn) {
          // 重复进入同一房间：回快照即可。
          fbs::ServerCharacterMessengerT m;
          m.step = 1;
          m.to_id = sn;
          m.x = static_cast<uint8_t>(FindSlot(old->second, client_id));
          FillSnapshot(m, old->second);
          SendToClient(client_id, m);
          return;
        }
        LeaveRoom(old, client_id);
      }
      if (sn == 0) {
        // 新建房间（095 TryNew(0)）。
        ServerMessengerRoom room;
        room.id = NextRoomId();
        room.client_ids.push_back(client_id);
        Rooms()[room.id] = room;
        fbs::ServerCharacterMessengerT m;
        m.step = 1;
        m.to_id = room.id;
        m.x = 0;
        FillSnapshot(m, Rooms().at(room.id));
        SendToClient(client_id, m);
        return;
      }
      auto it = FindRoomBySn(sn);
      // 房间不存在 / 已满：不能参与受招待的聊天房（095 String[820]）。
      if (it == Rooms().end() ||
          it->second.client_ids.size() >= ServerMessengerRoom::kMaxMembers) {
        fbs::ServerCharacterMessengerT m;
        m.step = 1;
        m.x = 255;
        SendToClient(client_id, m);
        return;
      }
      const int slot = static_cast<int>(it->second.client_ids.size());
      it->second.client_ids.push_back(client_id);
      // 本人：进入结果 + 全量快照（095 step1）。
      {
        fbs::ServerCharacterMessengerT m;
        m.step = 1;
        m.to_id = sn;
        m.x = static_cast<uint8_t>(slot);
        FillSnapshot(m, it->second);
        SendToClient(client_id, m);
      }
      // 其他人：成员进入（095 step0 OnEnter）。
      {
        fbs::ServerCharacterMessengerT m;
        m.step = 0;
        m.x = static_cast<uint8_t>(slot);
        auto player = LoadPlayer(client_id);
        if (player != nullptr) {
          m.player = std::move(player);
        }
        for (auto c_id : it->second.client_ids) {
          if (c_id != client_id) {
            SendToClient(c_id, m);
          }
        }
      }
      return;
    }
    // 离开房间（095 OnDestroy：CP 143 step2）。
    case 2: {
      auto it = FindClientRoom(client_id);
      if (it != Rooms().end()) {
        LeaveRoom(it, client_id);
      }
      return;
    }
    // 邀请聊天（095 SendInviteMsg：CP 143 step3）。
    case 3: {
      uint64_t target = r.to_id;
      if (target == 0) {
        std::u16string name{r.chat.begin(), r.chat.end()};
        target = FindClientByName(name);
      }
      if (target == client_id) {
        // 095 String[822]：请输入另外角色名。
        fbs::ServerCharacterMessengerT m;
        m.step = 4;
        m.x = 4;
        SendToClient(client_id, m);
        return;
      }
      if (!ClientAlive(target)) {
        // 095 String[793]：找不到'%s'玩家。
        fbs::ServerCharacterMessengerT m;
        m.step = 4;
        m.x = 0;
        m.player = LoadPlayer(target);
        SendToClient(client_id, m);
        return;
      }
      // 邀请人没房间时自动建房（095：TryNew(0) 后立即邀请）。
      auto it = FindClientRoom(client_id);
      if (it == Rooms().end()) {
        ServerMessengerRoom room;
        room.id = NextRoomId();
        room.client_ids.push_back(client_id);
        Rooms()[room.id] = room;
        it = Rooms().find(room.id);
        fbs::ServerCharacterMessengerT ack;
        ack.step = 1;
        ack.to_id = room.id;
        ack.x = 0;
        FillSnapshot(ack, it->second);
        SendToClient(client_id, ack);
      }
      auto& room = it->second;
      // 目标已在本房间。
      if (std::ranges::contains(room.client_ids, target)) {
        fbs::ServerCharacterMessengerT m;
        m.step = 4;
        m.x = 2;
        m.player = LoadPlayer(target);
        SendToClient(client_id, m);
        return;
      }
      // 房间已满。
      if (room.client_ids.size() >= ServerMessengerRoom::kMaxMembers) {
        fbs::ServerCharacterMessengerT m;
        m.step = 4;
        m.x = 3;
        m.player = LoadPlayer(target);
        SendToClient(client_id, m);
        return;
      }
      // 同一目标 30 秒内不重复邀请。
      auto& invites = Invites();
      if (std::ranges::any_of(invites, [&](const ServerMessengerInvite& inv) {
            return inv.to_client == target && inv.from_client == client_id &&
                   inv.expire > Window::dt_now_;
          })) {
        return;
      }
      ServerMessengerInvite inv;
      inv.from_client = client_id;
      inv.to_client = target;
      inv.sn = room.id;
      inv.expire = Window::dt_now_ + kInviteTimeoutMs;
      invites.push_back(inv);
      // 目标：邀请确认框（095 step3 OnInvite）。
      {
        fbs::ServerCharacterMessengerT m;
        m.step = 3;
        m.to_id = room.id;
        auto player = LoadPlayer(client_id);
        if (player != nullptr) {
          m.player = std::move(player);
        }
        SendToClient(target, m);
      }
      // 本人：邀请回执（095 String[792]）。
      {
        fbs::ServerCharacterMessengerT m;
        m.step = 4;
        m.x = 1;
        m.player = LoadPlayer(target);
        SendToClient(client_id, m);
      }
      return;
    }
    // 拒绝邀请（095 OnInvite 自动拒绝分支：CP 143 step5）。
    case 5: {
      auto& invites = Invites();
      std::erase_if(invites, [&](const ServerMessengerInvite& inv) {
        return inv.to_client == client_id &&
               inv.from_client == r.to_id;
      });
      fbs::ServerCharacterMessengerT m;
      m.step = 5;
      m.x = 0;  // 095 String[795]：'%s'玩家拒绝了邀请。
      auto player = LoadPlayer(client_id);
      if (player != nullptr) {
        m.player = std::move(player);
      }
      SendToClient(r.to_id, m);
      return;
    }
    // 聊天 / 打字状态（095 ProcessChat / Update）。
    case 6: {
      auto it = FindClientRoom(client_id);
      if (it == Rooms().end() ||
          !std::ranges::contains(it->second.client_ids, client_id)) {
        return;
      }
      fbs::ServerCharacterMessengerT m;
      m.step = 6;
      m.x = r.x;
      m.chat = r.chat;
      auto player = LoadPlayer(client_id);
      if (player != nullptr) {
        m.player = std::move(player);
      }
      // 095：本人已本地显示，只转发给其他人。
      for (auto c_id : it->second.client_ids) {
        if (c_id != client_id) {
          SendToClient(c_id, m);
        }
      }
      return;
    }
    default: {
      return;
    }
  }
}

bool ServerMessengerInstance::Run() {
  // 过期邀请。
  auto& invites = Invites();
  std::erase_if(invites, [](const ServerMessengerInvite& inv) {
    return inv.expire <= Window::dt_now_;
  });
  // 掉线成员按离房处理。
  for (auto it = Rooms().begin(); it != Rooms().end();) {
    if (PruneRoom(it)) {
      it = Rooms().begin();
      continue;
    }
    ++it;
  }
  return true;
}

void ServerMessengerInstance::HandleServer(
    uint64_t client_id, fbs::ServerCharacterMessengerT& r) {
  auto add_name = [&](std::u16string fmt, const fbs::PlayerT* p,
                      int color = 1) {
    std::u16string name = p != nullptr && p->character != nullptr
                              ? std::u16string{p->character->name.begin(),
                                               p->character->name.end()}
                              : std::u16string{u"???"};
    // 095 格式串里的 '%s'。
    auto pos = fmt.find(u"%s");
    if (pos != std::u16string::npos) {
      fmt.replace(pos, 2, name);
    }
    MapleChatUiSystem::AddChat(fmt, color);
  };

  switch (r.step) {
    // 成员进入（095 OnEnter）。
    case 0: {
      MapleChatUiSystem::OnMemberEnter(*r.player, r.x);
      return;
    }
    // 自己进入结果（095 OnSelfEnterResult）。
    case 1: {
      if (r.x == 255) {
        MapleChatUiSystem::EnterFailed();
        return;
      }
      MapleChatUiSystem::OnSelfEnter(r.to_id, r.x, r.players);
      return;
    }
    // 成员离开（095 OnLeave）。
    case 2: {
      MapleChatUiSystem::OnMemberLeave(r.player.get(), r.to_id, r.x);
      return;
    }
    // 邀请确认框（095 OnInvite → CUIFadeYesNo CreateMSMInvite）。
    case 3: {
      if (r.player == nullptr) {
        return;
      }
      PopupTipGameInstance::AddMapleChatTip(r.to_id, *r.player);
      return;
    }
    // 邀请回执（095 OnInviteResult）。
    case 4: {
      switch (r.x) {
        case 1:  // String[792]：- 向'%s'玩家传递了招待邀请。
          add_name(u"- 向'%s'玩家传递了招待邀请。", r.player.get());
          return;
        case 0:  // String[793]：- 找不到'%s'玩家。
          add_name(u"- 找不到'%s'玩家。", r.player.get(), 2);
          return;
        case 2:
          add_name(u"-%s玩家已在聊天室。", r.player.get());
          return;
        case 3:
          MapleChatUiSystem::AddChat(u"- 聊天室人员已满。", 2);
          return;
        case 4:  // String[822]：- 请输入另外角色名。
          MapleChatUiSystem::AddChat(u"- 请输入另外角色名。", 2);
          return;
        default:
          return;
      }
    }
    // 对方拒绝（095 OnBlocked）。
    case 5: {
      if (r.x == 1) {
        // String[794]：'%s'玩家现在是拒绝聊天状态。
        add_name(u"'%s'玩家现在是拒绝聊天状态。", r.player.get(), 2);
      } else {
        // String[795]：'%s'玩家拒绝了邀请。
        add_name(u"'%s'玩家拒绝了邀请。", r.player.get(), 2);
      }
      return;
    }
    // 聊天 / 打字状态（095 OnChat）。
    case 6: {
      std::u16string text{r.chat.begin(), r.chat.end()};
      MapleChatUiSystem::OnChat(r.player.get(), text, r.x);
      return;
    }
    default: {
      return;
    }
  }
}

}  // namespace sdlms
