#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_obj.h"
namespace sdlms {

class ObjRenderSystem {
public:
  static bool Render(GameObj &g_obj);
  static void RenderClock(SDL_FRect &r);
};

}  // namespace sdlms
