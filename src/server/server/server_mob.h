#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_mob.h"
#include <cstdint>
#include <optional>
#include <string>
class server_mob {
public:
  game_mob mob;
  uint64_t revive;//复活时间

  float hspeed;
  float vspeed;

  float hforce=1400;
};