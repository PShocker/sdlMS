#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_npc.h"
#include <string>
class NpcRenderSystem {
private:
  static void RenderNametag(GameNpc &g_npc);
  static void RenderQuest(GameNpc &g_npc);

public:
  static void RenderNametag(const std::u16string &text, SDL_FPoint p);
  static bool RenderNpc(GameNpc &g_npc);
  static bool Render(GameNpc &g_npc);
  static void RenderChatballoon(GameNpc &g_npc);
};