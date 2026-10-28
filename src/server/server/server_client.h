#pragma once

#include "src/common/flatbuffers/common.h"
#include "uv.h"
#include <cstdint>
namespace sdlms {


using namespace fbs;

struct ServerClient{
  uint64_t client_id;
  uint64_t map_id;

  PlayerT player_t;
  uint64_t heartbeat;
};

}  // namespace sdlms
