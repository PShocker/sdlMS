#pragma once

#include "src/common/flatbuffers/common.h"
#include "uv.h"
#include <cstdint>

class client_instance {
public:
  uint64_t id;
  sockaddr_in addr; // 客户端地址
  bool scene_host;
  uint64_t scene;
  uint64_t heartbeat;
  fbs::PlayerT fbs_player;
};