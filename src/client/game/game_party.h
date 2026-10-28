#pragma once

#include "SDL3/SDL_rect.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <string>
#include <vector>
namespace sdlms {



struct GameParty{
  uint64_t captain_id;
  std::vector<fbs::PlayerT> clients;
};

}  // namespace sdlms
