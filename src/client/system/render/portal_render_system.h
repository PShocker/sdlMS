#pragma once

#include "src/client/game/game_portal.h"
class portal_render_system {
private:
  static bool render_portal(game_portal &g_portal);
  static void render_nametag(game_portal &g_portal);

public:
  static bool render(game_portal &g_portal);
};