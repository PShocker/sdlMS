#pragma once

#include "src/client/game/game_drop.h"
#include "src/common/flatbuffers/common.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>

namespace sdlms {

struct ServerDrop {
  fbs::DropT dt;
  uint64_t available;
  uint64_t destroy;
};

}  // namespace sdlms
