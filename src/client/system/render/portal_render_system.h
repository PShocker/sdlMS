#pragma once

#include "src/client/game/game_portal.h"
class PortalRenderSystem {
private:
  static bool render_portal(GamePortal &g_portal);
  static void render_nametag(GamePortal &g_portal);

public:
  static bool render(GamePortal &g_portal);
};