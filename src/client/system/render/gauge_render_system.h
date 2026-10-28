#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_gauge.h"
#include <cstdint>
class GaugeRenderSystem {
public:
  static bool render_mob(SDL_FPoint pos, GameGauge &g);
  static bool render_character(SDL_FPoint pos, float hp_percent);
};