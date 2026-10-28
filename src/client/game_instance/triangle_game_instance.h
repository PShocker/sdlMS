#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_triangle.h"
namespace sdlms {

class TriangleGameInstance {
public:
  static bool RectInsTri(SDL_FRect &r, GameTriangle &t);
  static GameTriangle LoadTri(GameTriangle t, bool flip, SDL_FPoint &pos);
};

}  // namespace sdlms
