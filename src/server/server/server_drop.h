#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_drop.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <optional>

using namespace fbs;

class ServerDrop {
public:
  DropT dt;
  uint64_t available;
  uint64_t destroy;
};