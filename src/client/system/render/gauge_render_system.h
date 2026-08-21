#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_gauge.h"
#include <cstdint>
class gauge_render_system {
public:
  static bool render_mob(SDL_FPoint pos, game_gauge &g);
  static bool render_character(SDL_FPoint pos, float hp_percent);
};