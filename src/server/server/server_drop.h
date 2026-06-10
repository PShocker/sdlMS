#pragma once

#include "SDL3/SDL_rect.h"
#include "server_equip.h"
#include "src/client/game/game_drop.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <optional>

using namespace fbs;

class server_drop {
public:
  DropT drop;
};