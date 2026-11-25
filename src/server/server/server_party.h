#pragma once

#include <cstdint>
#include <vector>

namespace sdlms {

// 095 组队（PARTYDATA）：上限 6 人，队长（Boss）在首位。
struct ServerParty {
  uint64_t id = 0;
  uint64_t captain = 0;
  std::vector<uint64_t> client_ids;  // 含队长，队长在首位

  // 095：解散时全员收到变动；本实现用 id 递增区分不同队伍。
  static constexpr size_t kMaxMembers = 6;
};

// 待处理的入队邀请（095 PartyInvite 30 秒有效期）。
struct ServerPartyInvite {
  uint64_t from_captain = 0;  // 邀请发起人（必为队长）
  uint64_t to_client = 0;
  uint64_t expire = 0;        // Window::dt_now_ + 30s
};

}  // namespace sdlms