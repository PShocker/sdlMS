#pragma once

#include <cstdint>
#include <vector>

namespace sdlms {

// MapleChat 聊天室（095 CUIMessenger 的 Messenger 房间）。
// 成员顺序即 slot（客户端按 MapleChat.img/NameTag 0..3 显示）。
struct ServerMessengerRoom {
  uint64_t id = 0;
  std::vector<uint64_t> client_ids;  // 按加入顺序，下标=slot

  // 095 为 3 人房；本包 NameTag 有 0..3 四个名牌，取 4。
  static constexpr size_t kMaxMembers = 4;
};

// 待处理的聊天邀请（095 邀请 30 秒内可接受）。
struct ServerMessengerInvite {
  uint64_t from_client = 0;  // 邀请人
  uint64_t to_client = 0;    // 被邀请人
  uint64_t sn = 0;           // 邀请人所在房间 SN
  uint64_t expire = 0;       // Window::dt_now_ + 30s
};

}  // namespace sdlms
