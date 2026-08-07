#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_obj.h"
class obj_render_system {
public:
  static bool render(game_obj &g_obj);
  static void render_clock(SDL_FRect &r);
};