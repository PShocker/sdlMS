#pragma once

#include <cstdint>
namespace sdlms {

struct ServerHeartbeatInstance{
  static inline uint64_t host_heartbeat;
  const static inline uint8_t heartbeat_interval = 2;
};

}  // namespace sdlms
