#ifndef SDLMS_SRC_CLIENT_GAME_GAME_PARTY_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_PARTY_H_

#include "src/common/flatbuffers/common.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

struct GameParty {
  uint64_t captain_id;
  std::vector<fbs::PlayerT> clients;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_PARTY_H_
