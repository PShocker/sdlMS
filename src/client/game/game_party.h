#pragma once

#include "SDL3/SDL_rect.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <string>
#include <vector>
namespace sdlms {


using namespace fbs;

struct GameParty{
  uint64_t captain_id;
  std::vector<PlayerT> clients;
};

}  // namespace sdlms
