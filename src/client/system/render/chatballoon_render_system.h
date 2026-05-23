#pragma once

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_chatballoon.h"
#include <flat_map>
#include <string>
class chatballoon_render_system {
public:
  static bool render(game_chatballoon &g_chatballoon, SDL_FPoint base);
};