#pragma once

#include "src/client/game/game_npc.h"
class npc_render_system {
private:
  static void render_nametag(game_npc &g_character);
  static void render_chatballoon(game_npc &g_character);
  static bool render_npc(game_npc &g_npc);

public:
  static bool render(game_npc &g_npc);
};