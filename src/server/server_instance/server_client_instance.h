#pragma once

#include "src/server/server/server_client.h"

#include <cstdint>
#include <flat_map>
#include <string>

namespace sdlms {

class ServerClientInstance {
 public:
  static void ClientExit(uint64_t client_id);

  // 好友上线通知：服务端在某角色首次进图时广播给所有客户端，
  // 客户端按自己的好友列表（人物唯一 id）过滤后弹 poptip。
  static void BroadcastFriendOnline(uint64_t character_id,
                                    const std::u16string& name);

  static inline std::flat_map<uint64_t, ServerClient> clients_;
};

}  // namespace sdlms