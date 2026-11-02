#pragma once

#include <cstdint>

namespace sdlms {

struct ServerHeartbeatInstance {
  static inline uint64_t host_heartbeat;
  // 非主机客户端真正进入游戏场景后才开始发心跳。
  static inline bool client_started = false;
  // 2 秒一次心跳；连续约 5 次未收到即 10 秒判定断线。
  static constexpr uint32_t heartbeat_interval_ms = 2000;
  static constexpr uint32_t heartbeat_timeout_ms = 10000;
};

}  // namespace sdlms
