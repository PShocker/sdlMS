#pragma once

#include "src/common/flatbuffers/common.h"
#include "uv.h"
#include <cstdint>

class server_client {
public:
  uint64_t id;
  sockaddr_in addr; // 客户端地址
  uint64_t map_id;

  fbs::PlayerT fbs_player;
  uint64_t heartbeat;
};