#include "src/server/server_instance/server_minigame_instance.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/system/ui/minigame_card_ui_system.h"
#include "src/client/system/ui/minigame_omok_ui_system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_scene_instance.h"

#include <algorithm>
#include <memory>
#include <random>
#include <vector>

namespace sdlms {

namespace {

// 095 COmokDlg：15×15，每手 30 秒。
constexpr int kBoardSize = 15;
constexpr uint64_t kTurnTimeoutMs = 30 * 1000;
// 095 CMemoryGameDlg（GameKind=2）：30 张牌（15 对），每回合 30 秒。
constexpr int kCardCount = 30;

struct OmokRoom {
  uint32_t map_id = 0;
  // 房间类型：0=五子棋（0408 道具），1=卡片配对（040801 道具）。
  uint8_t game_type = 0;
  // 095 MiniRoom 私房密码；空 = 公开房（开房设置弹窗里不勾密码）。
  std::u16string password;
  uint64_t slot[2] = {0, 0};  // client_id，0 = 空位；slot0 = 房主(黑)
  bool guest_ready = false;
  bool playing = false;
  uint8_t board[kBoardSize][kBoardSize] = {};  // 0 空 1 黑 2 白
  std::vector<uint8_t> hx;                     // 落子历史（列）
  std::vector<uint8_t> hy;                     // 落子历史（行）
  std::vector<uint8_t> hc;                     // 落子历史（色）
  uint8_t turn = 0;          // 当前行棋色/行动者 slot
  uint64_t turn_start = 0;
  uint8_t req_kind = 0;      // 0 无 / 6 悔棋 / 8 求和
  uint64_t req_from = 0;
  uint32_t win[2] = {0, 0};
  uint32_t lose[2] = {0, 0};
  uint32_t draw[2] = {0, 0};
  // 卡片配对状态。
  uint8_t cards[kCardCount] = {};   // 牌面 0-14 成对；0xFF=已配对
  int first_pick = -1;              // 本回合第一张
  uint8_t card_score[2] = {0, 0};   // 配对数
};

std::vector<OmokRoom>& Rooms() {
  static std::vector<OmokRoom> rooms;
  return rooms;
}

int FindMember(const OmokRoom& room, uint64_t client_id) {
  for (int i = 0; i < 2; ++i) {
    if (room.slot[i] == client_id) {
      return i;
    }
  }
  return -1;
}

bool ClientAlive(uint64_t client_id) {
  return client_id != 0 && ServerClientInstance::clients_.contains(client_id);
}

std::unique_ptr<fbs::PlayerT> MakePlayer(uint64_t client_id) {
  return std::make_unique<fbs::PlayerT>(
      ServerClientInstance::clients_.at(client_id).player_t);
}

void SendTo(uint64_t client_id, fbs::ServerCharacterMinigameT& m) {
  if (ClientAlive(client_id)) {
    ServerResponse::SendToClient(client_id, m);
  }
}

fbs::ServerCharacterMinigameT MakeStep(uint8_t step) {
  fbs::ServerCharacterMinigameT m;
  m.step = step;
  return m;
}

// 四方向连五（095 悔棋以外的规则同样简单粗暴：无禁手）。
bool CheckWin(const OmokRoom& room) {
  static const int dx[4] = {1, 0, 1, 1};
  static const int dy[4] = {0, 1, 1, -1};
  const int px = room.hx.back();
  const int py = room.hy.back();
  const uint8_t color = room.hc.back();
  for (int d = 0; d < 4; ++d) {
    int count = 1;
    for (int dir = -1; dir <= 1; dir += 2) {
      int x = px + dx[d] * dir;
      int y = py + dy[d] * dir;
      while (x >= 0 && x < kBoardSize && y >= 0 && y < kBoardSize &&
             room.board[y][x] == color) {
        ++count;
        x += dx[d] * dir;
        y += dy[d] * dir;
      }
    }
    if (count >= 5) {
      return true;
    }
  }
  return false;
}

// 结算：winner_color 0/1 = 黑/白胜，2 = 平局（095 OnGameResult）。
void SendResult(OmokRoom& room, uint8_t winner_color) {
  room.playing = false;
  room.req_kind = 0;
  room.turn = 0;
  if (winner_color == 2) {
    ++room.draw[0];
    ++room.draw[1];
  } else if (winner_color == 0) {
    ++room.win[0];
    ++room.lose[1];
  } else {
    ++room.win[1];
    ++room.lose[0];
  }
  for (int i = 0; i < 2; ++i) {
    if (!ClientAlive(room.slot[i])) {
      continue;
    }
    fbs::ServerCharacterMinigameT m = MakeStep(5);
    m.x = winner_color;
    m.win = room.win[i];
    m.lose = room.lose[i];
    m.draw = room.draw[i];
    SendTo(room.slot[i], m);
  }
}

// 把 client_id 从房间移除。返回 true 表示房间已删除。
bool LeaveRoom(std::vector<OmokRoom>::iterator it, uint64_t client_id,
               int member_slot) {
  auto& room = *it;
  // 对局中途退房 = 认输（095 SendClaimGiveUp 同级处理）。
  if (room.playing) {
    SendResult(room, static_cast<uint8_t>(1 - member_slot));
  }
  room.slot[member_slot] = 0;
  const uint64_t other = room.slot[1 - member_slot];
  if (member_slot == 0) {
    // 房主退房：房间解散，客方收到离开。
    if (ClientAlive(other)) {
      fbs::ServerCharacterMinigameT m = MakeStep(2);
      SendTo(other, m);
    }
    Rooms().erase(it);
    return true;
  }
  // 客方退房：房主留在房间继续等待。
  if (ClientAlive(other)) {
    fbs::ServerCharacterMinigameT m = MakeStep(2);
    SendTo(other, m);
  }
  return false;
}

// 找到 client 所在房间，找不到返回 Rooms().end()。
std::vector<OmokRoom>::iterator FindMyRoom(uint64_t client_id) {
  auto& rooms = Rooms();
  for (auto it = rooms.begin(); it != rooms.end(); ++it) {
    if (FindMember(*it, client_id) >= 0) {
      return it;
    }
  }
  return rooms.end();
}

}  // namespace

void ServerMinigameInstance::HandleClient(uint64_t client_id,
                                          fbs::ClientCharacterMinigameT& r) {
  if (!ClientAlive(client_id)) {
    return;
  }
  auto& rooms = Rooms();

  switch (r.step) {
    // 进入房间（背包使用小游戏桌）。x：0=五子棋，1=卡片配对。
    case 0: {
      r.x = r.x > 1 ? 0 : r.x;
      if (FindMyRoom(client_id) != rooms.end()) {
        return;
      }
      // 找到玩家所在地图。
      uint32_t map_id = 0;
      for (const auto& [key, scene] : ServerSceneInstance::scenes_) {
        if (scene.clients.contains(client_id)) {
          map_id = scene.map_id;
          break;
        }
      }
      if (map_id == 0) {
        return;
      }
      auto it = std::ranges::find_if(rooms, [&](const OmokRoom& room) {
        return room.map_id == map_id && room.game_type == r.x;
      });
      if (it != rooms.end()) {
        auto& room = *it;
        if (room.playing || (room.slot[0] != 0 && room.slot[1] != 0)) {
          // 房间已满 / 正在对局。
          fbs::ServerCharacterMinigameT m = MakeStep(12);
          m.x = r.x;
          std::u16string text = u"房间里已经有人在对局了。";
          m.chat = {text.begin(), text.end()};
          SendTo(client_id, m);
          return;
        }
        // 私房：密码不匹配则拒绝进入。
        std::u16string input{r.chat.begin(), r.chat.end()};
        if (!room.password.empty() && input != room.password) {
          fbs::ServerCharacterMinigameT m = MakeStep(12);
          m.x = r.x;
          std::u16string text = u"密码不正确。";
          m.chat = {text.begin(), text.end()};
          SendTo(client_id, m);
          return;
        }
        // 进房：客方坐 slot1。
        room.slot[1] = client_id;
        room.guest_ready = false;
        fbs::ServerCharacterMinigameT to_owner = MakeStep(1);
        to_owner.x = r.x;
        to_owner.color = 0;  // 收包者的 slot：房主 0
        to_owner.player = MakePlayer(client_id);
        SendTo(room.slot[0], to_owner);
        fbs::ServerCharacterMinigameT to_guest = MakeStep(1);
        to_guest.x = r.x;
        to_guest.color = 1;  // 收包者的 slot：客方 1
        to_guest.player = MakePlayer(room.slot[0]);
        SendTo(client_id, to_guest);
        return;
      }
      // 建房：房主坐 slot0；chat 带入开房设置的密码（空 = 公开房）。
      OmokRoom room;
      room.map_id = map_id;
      room.game_type = r.x;
      room.password = {r.chat.begin(), r.chat.end()};
      room.slot[0] = client_id;
      rooms.push_back(room);
      fbs::ServerCharacterMinigameT m = MakeStep(0);
      m.x = r.x;
      SendTo(client_id, m);
      return;
    }
    // 准备（仅客方）。
    case 1: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end() || it->playing) {
        return;
      }
      int slot = FindMember(*it, client_id);
      if (slot != 1 || it->slot[0] == 0) {
        return;
      }
      it->guest_ready = true;
      fbs::ServerCharacterMinigameT m = MakeStep(6);
      m.color = 1;
      SendTo(it->slot[0], m);
      return;
    }
    // 开始（仅房主，需对方已准备）。
    case 2: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end() || it->playing) {
        return;
      }
      int slot = FindMember(*it, client_id);
      if (slot != 0 || it->slot[1] == 0 || !it->guest_ready) {
        return;
      }
      it->playing = true;
      it->guest_ready = false;
      it->req_kind = 0;
      it->turn_start = Window::dt_now_;

      if (it->game_type == 1) {
        // 卡片配对：发牌（0-14 成对洗牌，095 OnUserStart shuffle）。
        for (int i = 0; i < kCardCount; ++i) {
          it->cards[i] = static_cast<uint8_t>(i / 2);
        }
        std::shuffle(std::begin(it->cards), std::end(it->cards),
                     std::default_random_engine{
                         static_cast<unsigned>(Window::dt_now_)});
        it->first_pick = -1;
        it->card_score[0] = 0;
        it->card_score[1] = 0;
        it->turn = 0;  // 房主先翻
        for (int i = 0; i < 2; ++i) {
          fbs::ServerCharacterMinigameT m = MakeStep(3);
          m.x = it->turn;  // 先手 slot
          m.y = kCardCount;
          m.color = static_cast<uint8_t>(i);  // 本人 slot
          m.cards.assign(it->cards, it->cards + kCardCount);
          SendTo(it->slot[i], m);
        }
        return;
      }

      // 五子棋。
      for (auto& row : it->board) {
        for (auto& cell : row) {
          cell = 0;
        }
      }
      it->hx.clear();
      it->hy.clear();
      it->hc.clear();
      it->turn = 0;
      for (int i = 0; i < 2; ++i) {
        fbs::ServerCharacterMinigameT m = MakeStep(3);
        m.color = static_cast<uint8_t>(i);  // slot0 黑 / slot1 白
        SendTo(it->slot[i], m);
      }
      return;
    }
    // 落子 / 翻卡。
    case 4: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end() || !it->playing) {
        return;
      }
      int slot = FindMember(*it, client_id);
      if (slot < 0) {
        return;
      }

      // 卡片配对：x=卡索引（095 SendTurnUpCard）。
      if (it->game_type == 1) {
        if (slot != it->turn || r.x >= kCardCount || it->cards[r.x] == 0xFF) {
          return;
        }
        it->turn_start = Window::dt_now_;
        if (it->first_pick < 0) {
          // 第一张：广播翻开。
          it->first_pick = r.x;
          for (int i = 0; i < 2; ++i) {
            fbs::ServerCharacterMinigameT m = MakeStep(13);
            m.x = r.x;
            m.y = it->cards[r.x];
            m.color = it->turn;
            SendTo(it->slot[i], m);
          }
          return;
        }
        // 第二张：判定配对（095 OnTurnUpCard）。
        const int first = it->first_pick;
        it->first_pick = -1;
        for (int i = 0; i < 2; ++i) {
          fbs::ServerCharacterMinigameT m = MakeStep(13);
          m.x = r.x;
          m.y = it->cards[r.x];
          m.color = it->turn;
          SendTo(it->slot[i], m);
        }
        if (it->cards[first] == it->cards[r.x]) {
          // 配对成功：加分，继续翻（095 匹配不换手）。
          it->cards[first] = 0xFF;
          it->cards[r.x] = 0xFF;
          ++it->card_score[slot];
          int remain = 0;
          for (int i = 0; i < kCardCount; ++i) {
            if (it->cards[i] != 0xFF) {
              ++remain;
            }
          }
          if (remain == 0) {
            SendResult(*it, it->card_score[0] == it->card_score[1]
                                ? 2
                                : static_cast<uint8_t>(
                                      it->card_score[0] > it->card_score[1]
                                          ? 0
                                          : 1));
            return;
          }
          for (int i = 0; i < 2; ++i) {
            fbs::ServerCharacterMinigameT m = MakeStep(14);
            m.x = static_cast<uint8_t>(first);
            m.y = r.x;
            m.color = static_cast<uint8_t>(slot);  // 配对者
            m.chat = {static_cast<uint16_t>(it->card_score[0]),
                      static_cast<uint16_t>(it->card_score[1])};
            SendTo(it->slot[i], m);
          }
          return;
        }
        // 配对失败：延时翻回 + 换手。
        it->turn = static_cast<uint8_t>(1 - slot);
        for (int i = 0; i < 2; ++i) {
          fbs::ServerCharacterMinigameT m = MakeStep(15);
          m.x = static_cast<uint8_t>(first);
          m.y = r.x;
          m.color = it->turn;
          SendTo(it->slot[i], m);
        }
        return;
      }

      // 五子棋：x=列 y=行。
      if (slot != it->turn) {
        return;
      }
      if (r.x >= kBoardSize || r.y >= kBoardSize ||
          it->board[r.y][r.x] != 0) {
        return;
      }
      const uint8_t color = static_cast<uint8_t>(slot + 1);
      it->board[r.y][r.x] = color;
      it->hx.push_back(r.x);
      it->hy.push_back(r.y);
      it->hc.push_back(color);
      if (it->hx.size() == kBoardSize * kBoardSize) {
        // 下满未分胜负：平局。
        SendResult(*it, 2);
        return;
      }
      if (CheckWin(*it)) {
        SendResult(*it, static_cast<uint8_t>(slot));
        return;
      }
      it->turn = static_cast<uint8_t>(1 - slot);
      it->turn_start = Window::dt_now_;
      for (int i = 0; i < 2; ++i) {
        fbs::ServerCharacterMinigameT m = MakeStep(4);
        m.x = r.x;
        m.y = r.y;
        m.color = static_cast<uint8_t>(slot);
        SendTo(it->slot[i], m);
      }
      return;
    }
    // 房间聊天：广播给双方。
    case 5: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end() || r.chat.empty()) {
        return;
      }
      for (int i = 0; i < 2; ++i) {
        if (it->slot[i] == 0 || it->slot[i] == client_id) {
          continue;
        }
        fbs::ServerCharacterMinigameT m = MakeStep(7);
        m.player = MakePlayer(client_id);
        m.chat = r.chat;
        SendTo(it->slot[i], m);
      }
      return;
    }
    // 悔棋请求。
    case 6: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end() || !it->playing || it->req_kind != 0) {
        return;
      }
      int slot = FindMember(*it, client_id);
      if (slot < 0) {
        return;
      }
      it->req_kind = 6;
      it->req_from = client_id;
      fbs::ServerCharacterMinigameT m = MakeStep(8);
      m.player = MakePlayer(client_id);
      SendTo(it->slot[1 - slot], m);
      return;
    }
    // 悔棋答复。
    case 7: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end() || it->req_kind != 6) {
        return;
      }
      if (it->req_from == client_id) {
        return;  // 只有对方能答复
      }
      int req_slot = FindMember(*it, it->req_from);
      it->req_kind = 0;
      it->req_from = 0;
      if (r.x == 1 && req_slot >= 0) {
        // 撤回到请求者上一手之前（095 OnRetreatResult）。
        int last = -1;
        for (int i = static_cast<int>(it->hc.size()) - 1; i >= 0; --i) {
          if (it->hc[i] == static_cast<uint8_t>(req_slot + 1)) {
            last = i;
            break;
          }
        }
        if (last < 0) {
          fbs::ServerCharacterMinigameT m = MakeStep(9);
          m.x = 0;
          SendTo(it->slot[req_slot], m);
          return;
        }
        int count = static_cast<int>(it->hc.size()) - 1 - last;
        for (int i = 0; i < count; ++i) {
          it->board[it->hy.back()][it->hx.back()] = 0;
          it->hx.pop_back();
          it->hy.pop_back();
          it->hc.pop_back();
        }
        it->turn = static_cast<uint8_t>(req_slot);
        it->turn_start = Window::dt_now_;
        for (int i = 0; i < 2; ++i) {
          fbs::ServerCharacterMinigameT m = MakeStep(9);
          m.x = 1;
          m.y = static_cast<uint8_t>(count);
          m.color = static_cast<uint8_t>(req_slot);
          SendTo(it->slot[i], m);
        }
      } else {
        fbs::ServerCharacterMinigameT m = MakeStep(9);
        m.x = 0;
        SendTo(it->slot[req_slot], m);
      }
      return;
    }
    // 求和请求。
    case 8: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end() || !it->playing || it->req_kind != 0) {
        return;
      }
      int slot = FindMember(*it, client_id);
      if (slot < 0) {
        return;
      }
      it->req_kind = 8;
      it->req_from = client_id;
      fbs::ServerCharacterMinigameT m = MakeStep(10);
      m.player = MakePlayer(client_id);
      SendTo(it->slot[1 - slot], m);
      return;
    }
    // 求和答复。
    case 9: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end() || it->req_kind != 8) {
        return;
      }
      if (it->req_from == client_id) {
        return;
      }
      uint64_t requester = it->req_from;
      it->req_kind = 0;
      it->req_from = 0;
      if (r.x == 1) {
        fbs::ServerCharacterMinigameT m = MakeStep(11);
        m.x = 1;
        SendTo(requester, m);
        SendResult(*it, 2);
      } else {
        fbs::ServerCharacterMinigameT m = MakeStep(11);
        m.x = 0;
        SendTo(requester, m);
      }
      return;
    }
    // 认输。
    case 10: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end() || !it->playing) {
        return;
      }
      int slot = FindMember(*it, client_id);
      if (slot < 0) {
        return;
      }
      SendResult(*it, static_cast<uint8_t>(1 - slot));
      return;
    }
    // 离开。
    case 3: {
      auto it = FindMyRoom(client_id);
      if (it == rooms.end()) {
        return;
      }
      int slot = FindMember(*it, client_id);
      if (slot < 0) {
        return;
      }
      LeaveRoom(it, client_id, slot);
      return;
    }
    default: {
      return;
    }
  }
}

bool ServerMinigameInstance::Run() {
  auto& rooms = Rooms();
  for (auto it = rooms.begin(); it != rooms.end();) {
    auto& room = *it;
    // 掉线清理：成员不在服务端名单时按离房处理。
    bool erased = false;
    for (int slot = 0; slot < 2 && !erased; ++slot) {
      if (room.slot[slot] != 0 && !ClientAlive(room.slot[slot])) {
        erased = LeaveRoom(it, room.slot[slot], slot);
        break;
      }
    }
    if (erased) {
      // LeaveRoom 可能删除了当前房间，从头重扫。
      it = rooms.begin();
      continue;
    }
    // 超时判负（095 m_nTimeLeft = 30 秒）。
    if (room.playing && Window::dt_now_ - room.turn_start >= kTurnTimeoutMs) {
      SendResult(room, static_cast<uint8_t>(1 - room.turn));
    }
    ++it;
  }
  return true;
}

void ServerMinigameInstance::HandleServer(uint64_t client_id,
                                          fbs::ServerCharacterMinigameT& r) {
  using M = MinigameOmokUiSystem;
  using C = MinigameCardUiSystem;
  // x 携带游戏类型（0=五子棋 1=卡片配对）的 step 走对应 UI。
  auto is_card = [](const fbs::ServerCharacterMinigameT& m) {
    return m.x == 1;
  };

  switch (r.step) {
    // 房间建立：你是房主，等待对手。
    case 0: {
      if (is_card(r)) {
        C::phase_ = C::Phase::kWaiting;
        C::my_slot_ = 0;
        C::has_peer_ = false;
        C::peer_ready_ = false;
        C::my_ready_ = false;
        C::my_win_ = C::my_lose_ = C::my_draw_ = 0;
        C::chat_log_.clear();
        C::ResetRound();
        C::Open();
        C::AddChat(u"已进入配对卡房间，等待对手。");
        return;
      }
      M::phase_ = M::Phase::kWaiting;
      M::my_slot_ = 0;
      M::my_color_ = 0;
      M::has_peer_ = false;
      M::peer_ready_ = false;
      M::my_ready_ = false;
      M::my_win_ = M::my_lose_ = M::my_draw_ = 0;
      M::chat_log_.clear();
      M::ResetRound();
      M::Open();
      M::AddChat(u"已进入五子棋房间，等待对手。");
      return;
    }
    // 对手进入。
    case 1: {
      if (r.player == nullptr) {
        return;
      }
      if (is_card(r)) {
        C::peer_player_ = *r.player;
        C::peer_ =
            ServerCharacterInstance::LoadGCharacter(r.player->character);
        C::peer_name_ = C::peer_.name;
        M::PrepareShowCharacter(C::peer_);
        C::has_peer_ = true;
        C::peer_ready_ = false;
        C::my_ready_ = false;
        C::my_slot_ = r.color;  // 服务端带本人的 slot（10/1）
        C::phase_ = C::Phase::kReady;
        C::ResetRound();
        C::Open();
        C::AddChat(C::peer_name_ + u" 进入了房间。");
        AudioGameInstance::PlayNow(u"MiniGame.img/Enter");
        return;
      }
      M::peer_player_ = *r.player;
      M::peer_ =
          ServerCharacterInstance::LoadGCharacter(r.player->character);
      M::peer_name_ = M::peer_.name;
      M::PrepareShowCharacter(M::peer_);
      M::has_peer_ = true;
      M::peer_ready_ = false;
      M::my_ready_ = false;
      M::my_slot_ = r.color;  // 服务端带本人的 slot（10/1）
      M::phase_ = M::Phase::kReady;
      M::ResetRound();
      M::Open();
      M::AddChat(M::peer_name_ + u" 进入了房间。");
      AudioGameInstance::PlayNow(u"MiniGame.img/Enter");
      return;
    }
    // 对手离开 / 房间解散。
    case 2: {
      // 两个 UI 已开着的那个收下。
      if (C::phase_ != C::Phase::kIdle && M::phase_ == M::Phase::kIdle) {
        if (C::my_slot_ == 1) {
          C::phase_ = C::Phase::kIdle;
          C::Close();
          return;
        }
        C::has_peer_ = false;
        C::peer_ready_ = false;
        C::my_ready_ = false;
        C::phase_ = C::Phase::kWaiting;
        C::ResetRound();
        C::AddChat(u"对方离开了房间。");
        return;
      }
      if (M::my_slot_ == 1) {
        // 房主离开，房间解散。
        M::phase_ = M::Phase::kIdle;
        M::Close();
        return;
      }
      M::has_peer_ = false;
      M::peer_ready_ = false;
      M::my_ready_ = false;
      M::phase_ = M::Phase::kWaiting;
      M::ResetRound();
      M::AddChat(u"对方离开了房间。");
      return;
    }
    // 开局（五子棋：color=本人棋色；卡片：cards=牌面，x=先手，color=本人slot）。
    case 3: {
      if (C::phase_ != C::Phase::kIdle && M::phase_ == M::Phase::kIdle &&
          !r.cards.empty()) {
        // 卡片配对开局。
        C::my_slot_ = r.color;
        C::phase_ = C::Phase::kPlaying;
        C::my_ready_ = false;
        C::peer_ready_ = false;
        C::ResetRound();
        // ResetRound 会清 my_turn_/cards_，重置后再落数据。
        for (int i = 0; i < C::kCardCount && i < (int)r.cards.size(); ++i) {
          C::cards_[i] = r.cards[i];
          C::face_[i] = C::Face::kBack;
        }
        C::my_turn_ = r.x == r.color;
        // 095 OnUserStart→Update m_bShowAndHide：开局预览全部牌面。
        C::preview_until_ = Window::dt_now_ + 3000;
        C::turn_deadline_ = Window::dt_now_ + kTurnTimeoutMs;
        C::AddChat(C::my_turn_ ? u"对局开始，你先翻。" : u"对局开始，对方先翻。");
        AudioGameInstance::PlayNow(u"MiniGame.img/Enter");
        return;
      }
      M::my_color_ = r.color;
      M::turn_color_ = 0;
      M::phase_ = M::Phase::kPlaying;
      M::my_ready_ = false;
      M::peer_ready_ = false;
      M::ResetRound();
      M::turn_deadline_ = Window::dt_now_ + kTurnTimeoutMs;
      M::AddChat(M::my_color_ == 0 ? u"对局开始，你执黑先行。"
                                   : u"对局开始，对方执黑先行。");
      AudioGameInstance::PlayNow(u"MiniGame.img/Enter");
      return;
    }
    // 落子 / 卡片翻开一张（卡片走 step 13）。
    case 4: {
      if (C::phase_ == C::Phase::kPlaying && M::phase_ == M::Phase::kIdle) {
        return;  // 卡片游戏不用 step 4
      }
      if (r.x >= M::kBoardSize || r.y >= M::kBoardSize) {
        return;
      }
      M::board_[r.y][r.x] = static_cast<uint8_t>(r.color + 1);
      M::history_.push_back({r.x, r.y, static_cast<uint8_t>(r.color + 1)});
      M::last_stone_[0] = r.x;
      M::last_stone_[1] = r.y;
      M::turn_color_ = 1 - r.color;
      M::turn_deadline_ = Window::dt_now_ + kTurnTimeoutMs;
      // 落子音效（095：黑白各自音色）。
      AudioGameInstance::PlayNow(r.color == 0 ? u"MiniGame.img/Mushroom"
                                              : u"MiniGame.img/Slime");
      return;
    }
    // 结果（x：0/1 胜方颜色，2 平局；win/lose/draw 为本方战绩）。
    case 5: {
      if (C::phase_ != C::Phase::kIdle && M::phase_ == M::Phase::kIdle) {
        C::my_win_ = r.win;
        C::my_lose_ = r.lose;
        C::my_draw_ = r.draw;
        if (r.x == 2) {
          C::result_ = 2;
          C::AddChat(u"平局。");
          AudioGameInstance::PlayNow(u"MiniGame.img/Draw");
        } else {
          C::result_ = r.x == C::my_slot_ ? 0 : 1;
          C::AddChat(C::result_ == 0 ? u"你赢了。" : u"你输了。");
          AudioGameInstance::PlayNow(C::result_ == 0 ? u"MiniGame.img/Win"
                                                     : u"MiniGame.img/Loose");
        }
        C::phase_ = C::Phase::kResult;
        C::my_ready_ = false;
        C::peer_ready_ = false;
        C::draw_pending_ = false;
        return;
      }
      M::my_win_ = r.win;
      M::my_lose_ = r.lose;
      M::my_draw_ = r.draw;
      M::result_winner_color_ = r.x;
      if (r.x == 2) {
        M::result_ = 2;
        M::AddChat(u"平局。");
        AudioGameInstance::PlayNow(u"MiniGame.img/Draw");
      } else {
        M::result_ = r.x == M::my_color_ ? 0 : 1;
        M::AddChat(M::result_ == 0 ? u"你赢了。" : u"你输了。");
        AudioGameInstance::PlayNow(M::result_ == 0 ? u"MiniGame.img/Win"
                                                   : u"MiniGame.img/Loose");
      }
      M::phase_ = M::Phase::kResult;
      M::my_ready_ = false;
      M::peer_ready_ = false;
      M::undo_pending_ = false;
      M::draw_pending_ = false;
      return;
    }
    // 对方就绪状态。
    case 6: {
      if (C::phase_ != C::Phase::kIdle && M::phase_ == M::Phase::kIdle) {
        C::peer_ready_ = r.color == 1;
        if (C::peer_ready_) {
          C::AddChat(u"对方已准备。");
          AudioGameInstance::PlayNow(u"MiniGame.img/Ready");
        }
        return;
      }
      M::peer_ready_ = r.color == 1;
      if (M::peer_ready_) {
        M::AddChat(u"对方已准备。");
        AudioGameInstance::PlayNow(u"MiniGame.img/Ready");
      }
      return;
    }
    // 对方聊天。
    case 7: {
      auto* chat_target = (C::phase_ != C::Phase::kIdle &&
                           M::phase_ == M::Phase::kIdle)
                              ? &C::chat_log_
                              : &M::chat_log_;
      if (r.player != nullptr && r.player->character != nullptr) {
        std::u16string name{r.player->character->name.begin(),
                            r.player->character->name.end()};
        chat_target->push_back(name + u" : " +
                               std::u16string{r.chat.begin(), r.chat.end()});
        if (chat_target->size() > 100) {
          chat_target->erase(chat_target->begin());
        }
      }
      return;
    }
    // 悔棋请求（FadeYesNo 确认框，095 OnRetreatRequest）。
    case 8: {
      if (r.player != nullptr) {
        PopupTipGameInstance::AddOmokTip(6, *r.player);
      }
      return;
    }
    // 悔棋结果（x=1：y=移除数，color=回转后行棋方）。
    case 9: {
      if (r.x == 1) {
        int count = r.y;
        for (int i = 0; i < count && !M::history_.empty(); ++i) {
          auto& st = M::history_.back();
          M::board_[st.y][st.x] = 0;
          M::history_.pop_back();
        }
        M::last_stone_[0] = -1;
        M::last_stone_[1] = -1;
        if (!M::history_.empty()) {
          M::last_stone_[0] = M::history_.back().x;
          M::last_stone_[1] = M::history_.back().y;
        }
        M::turn_color_ = r.color;
        M::turn_deadline_ = Window::dt_now_ + kTurnTimeoutMs;
        M::undo_pending_ = false;
        M::AddChat(u"悔棋成功。");
      } else {
        M::undo_pending_ = false;
        M::AddChat(u"对方拒绝了你的悔棋请求。");
      }
      return;
    }
    // 求和请求（095 OnTieRequest）。
    case 10: {
      if (r.player != nullptr) {
        PopupTipGameInstance::AddOmokTip(8, *r.player);
      }
      return;
    }
    // 求和答复。
    case 11: {
      if (C::phase_ != C::Phase::kIdle && M::phase_ == M::Phase::kIdle) {
        C::draw_pending_ = false;
        if (r.x == 0) {
          C::AddChat(u"对方拒绝了求和。");
        }
        return;
      }
      M::draw_pending_ = false;
      if (r.x == 0) {
        M::AddChat(u"对方拒绝了求和。");
      }
      return;
    }
    // 进入失败提示（chat 带服务器文本，如"密码不正确。"）。
    case 12: {
      if (!r.chat.empty()) {
        NoticeUiSystem::notice_text_ = {r.chat.begin(), r.chat.end()};
        NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopNotice;
      } else {
        NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kMinigameRoomFull;
      }
      NoticeUiSystem::Open();
      return;
    }
    // 卡片翻开一张（x=卡索引，y=牌面，color=行动者 slot）。
    case 13: {
      if (r.x >= C::kCardCount) {
        return;
      }
      C::face_[r.x] = C::Face::kOpen;
      // 行动者不变（095 翻两张内不换手）；picked_ 由本地点击维护，
      // 14/15 判定后重置。
      C::my_turn_ = r.color == C::my_slot_;
      C::turn_deadline_ = Window::dt_now_ + kTurnTimeoutMs;
      // 095 翻卡音效。
      AudioGameInstance::PlayNow(u"MiniGame.img/Enter");
      return;
    }
    // 卡片配对成功（x,y=两张卡索引；chat[0]=slot0分数 chat[1]=slot1分数）。
    case 14: {
      if (r.x < C::kCardCount) {
        C::face_[r.x] = C::Face::kMatched;
        C::match_fx_at_[r.x] = Window::dt_now_;  // 095 DrawEffect
      }
      if (r.y < C::kCardCount) {
        C::face_[r.y] = C::Face::kMatched;
        C::match_fx_at_[r.y] = Window::dt_now_;
      }
      if (r.chat.size() >= 2) {
        C::pair_score_[0] = static_cast<uint8_t>(r.chat[0]);
        C::pair_score_[1] = static_cast<uint8_t>(r.chat[1]);
      }
      C::picked_ = 0;
      // 095 配对成功音效（Catch 风格选中音）。
      AudioGameInstance::PlayNow(u"MiniGame.img/Catch");
      return;
    }
    // 卡片配对失败（x,y=两张卡索引，color=换手后行动者）。
    case 15: {
      if (r.x < C::kCardCount) {
        C::face_[r.x] = C::Face::kOpen;
      }
      if (r.y < C::kCardCount) {
        C::face_[r.y] = C::Face::kOpen;
      }
      C::flip_back_[0] = r.x;
      C::flip_back_[1] = r.y;
      C::flip_back_at_ = Window::dt_now_ + 1000;  // 095 延时翻回
      C::my_turn_ = r.color == C::my_slot_;
      C::picked_ = 0;
      C::turn_deadline_ = Window::dt_now_ + kTurnTimeoutMs;
      // 095 不匹配音效 NoMatch。
      AudioGameInstance::PlayNow(u"MiniGame.img/NoMatch");
      return;
    }
    default: {
      return;
    }
  }
}

}  // namespace sdlms