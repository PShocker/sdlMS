#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_DROP_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_DROP_RENDER_SYSTEM_H_

#include "src/client/game/game_drop.h"
namespace sdlms {

class DropRenderSystem {
 public:
  static bool Render(GameDrop& g_drop);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_DROP_RENDER_SYSTEM_H_
