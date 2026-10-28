#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_npc.h"
#include <string>
class NpcRenderSystem {
private:
  static void render_nametag(GameNpc &g_npc);
  static void render_quest(GameNpc &g_npc);

public:
  static void render_nametag(const std::u16string &text, SDL_FPoint p);
  static bool render_npc(GameNpc &g_npc);
  static bool render(GameNpc &g_npc);
  static void render_chatballoon(GameNpc &g_npc);
};