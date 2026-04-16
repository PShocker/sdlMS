#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_nametag.h"
#include <string>
class nametag_render_system {
public:
  static bool render(game_nametag &g_nametag, SDL_FPoint base);
};