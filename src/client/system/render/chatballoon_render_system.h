#pragma once

#include "src/client/game/game_chatballoon.h"
class chatballoon_render_system {
public:
  static bool render(game_chatballoon &g_chatballoon, SDL_FPoint base);
};