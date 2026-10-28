#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_foothold.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
namespace sdlms {

class ServerFoothold {
public:
  GameFoothold fh;
};

}  // namespace sdlms
