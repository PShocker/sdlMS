#pragma once

#include "src/client/game/game_foothold.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>

namespace sdlms {

struct ServerFoothold {
  GameFoothold fh;
};

}  // namespace sdlms
