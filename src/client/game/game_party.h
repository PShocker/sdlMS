#pragma once

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
