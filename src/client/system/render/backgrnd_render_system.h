#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_BACKGRND_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_BACKGRND_RENDER_SYSTEM_H_

#include "src/client/game/game_backgrnd.h"
namespace sdlms {

class BackgrndRenderSystem {
public:
  static bool Render(GameBackgrnd &g_backgrnd);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_BACKGRND_RENDER_SYSTEM_H_
