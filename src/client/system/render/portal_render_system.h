#pragma once

#include "src/client/game/game_portal.h"
namespace sdlms {

class PortalRenderSystem {
private:
  static bool RenderPortal(GamePortal &g_portal);
  static void RenderNametag(GamePortal &g_portal);

public:
  static bool Render(GamePortal &g_portal);
};

}  // namespace sdlms
