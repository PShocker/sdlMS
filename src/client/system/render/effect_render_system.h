#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_effect.h"
class effect_render_system {
private:
  static void render_custom(SDL_FPoint pos, game_effect &g_effect);
  static void render_normal(SDL_FPoint pos, game_effect &g_effect);

public:
  static bool render(SDL_FPoint pos, game_effect &g_effect);
};