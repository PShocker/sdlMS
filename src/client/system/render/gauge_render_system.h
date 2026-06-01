#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_gauge.h"
#include <cstdint>
class gauge_render_system {
public:
  static bool render(SDL_FPoint pos, game_gauge &g, int64_t new_val);
};