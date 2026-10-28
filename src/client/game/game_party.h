#pragma once

#include "SDL3/SDL_rect.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <string>
#include <vector>

using namespace fbs;

class GameParty {
public:
  uint64_t captain_id;
  std::vector<PlayerT> clients;
};