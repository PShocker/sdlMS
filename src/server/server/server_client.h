#pragma once

#include "src/common/flatbuffers/common.h"
#include "uv.h"
#include <cstdint>

class server_client {
public:
  uint64_t client_id;
  uint64_t map_id;

  fbs::PlayerT fbs_player;
  uint64_t heartbeat;
};