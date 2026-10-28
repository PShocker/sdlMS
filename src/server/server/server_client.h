#ifndef SDLMS_SRC_SERVER_SERVER_SERVER_CLIENT_H_
#define SDLMS_SRC_SERVER_SERVER_SERVER_CLIENT_H_

#include "src/common/flatbuffers/common.h"
#include <uv.h>
#include <cstdint>
namespace sdlms {



struct ServerClient{
  uint64_t client_id;
  uint64_t map_id;

  fbs::PlayerT player_t;
  uint64_t heartbeat;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SERVER_CLIENT_H_
