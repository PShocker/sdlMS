#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_obj.h"
class ObjRenderSystem {
public:
  static bool render(GameObj &g_obj);
  static void render_clock(SDL_FRect &r);
};