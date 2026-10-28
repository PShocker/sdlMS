#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_REACTOR_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_REACTOR_RENDER_SYSTEM_H_

#include "src/client/game/game_reactor.h"

namespace sdlms {

class ReactorRenderSystem {
 public:
  static bool Render(GameReactor& g_reactor);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_REACTOR_RENDER_SYSTEM_H_
