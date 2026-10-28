#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_OBJ_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_OBJ_RENDER_SYSTEM_H_

#include "src/client/game/game_obj.h"

#include <SDL3/SDL_rect.h>

namespace sdlms {

class ObjRenderSystem {
 public:
  static bool Render(GameObj& g_obj);
  static void RenderClock(SDL_FRect& r);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_OBJ_RENDER_SYSTEM_H_
