#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_npc.h"
#include <string>
class npc_render_system {
private:
  static void render_nametag(game_npc &g_npc);
  static void render_quest(game_npc &g_npc);

public:
  static void render_nametag(const std::u16string &text, SDL_FPoint p);
  static bool render_npc(game_npc &g_npc);
  static bool render(game_npc &g_npc);
  static void render_chatballoon(game_npc &g_npc);
};