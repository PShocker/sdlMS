#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_triangle.h"
class TriangleGameInstance {
public:
  static bool rect_ins_tri(SDL_FRect &r, GameTriangle &t);
  static GameTriangle load_tri(GameTriangle t, bool flip, SDL_FPoint &pos);
};