#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_gain_log.h"
#include "src/client/game/game_gauge.h"
#include <cstdint>
class gain_log_render_system {
public:
  static void render(game_gain_log &g_log, uint32_t i);
};