#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_triangle.h"
class triangle_game_instance {
public:
  static bool rect_ins_tri(SDL_FRect &r, game_triangle &t);
  static game_triangle load_tri(game_triangle t, bool flip, SDL_FPoint &pos);
};