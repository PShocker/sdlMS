#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_PORTAL_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_PORTAL_RENDER_SYSTEM_H_

#include "src/client/game/game_portal.h"

namespace sdlms {

class PortalRenderSystem {
 private:
  static bool RenderPortal(GamePortal& g_portal);
  static void RenderNametag(GamePortal& g_portal);

 public:
  static bool Render(GamePortal& g_portal);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_PORTAL_RENDER_SYSTEM_H_
