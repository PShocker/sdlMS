#pragma once

#include <cstdint>
class server_heartbeat_instance {
public:
  static inline uint64_t host_heartbeat;
  const static uint8_t heartbeat_interval = 5;
};
