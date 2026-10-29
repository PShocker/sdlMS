#pragma once

#include "src/client/game/game_obj.h"

#include <SDL3/SDL_rect.h>

namespace sdlms {

class ObjRenderSystem {
 public:
  static bool Render(GameObj& g_obj);
  static void RenderClock(SDL_FRect& r);
};

}  // namespace sdlms
