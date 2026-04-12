#pragma once

#include "uv.h"
#include <cstdint>

class client_instance {
public:
  uint64_t id;
  sockaddr_in addr; // 客户端地址
  bool scene_host;
  uint64_t scene;
  uint64_t heartbeat;
  uv_timer_t heartbeat_timer;
};