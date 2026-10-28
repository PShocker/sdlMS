#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_TRIANGLE_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_TRIANGLE_GAME_INSTANCE_H_

#include "src/client/game/game_triangle.h"

#include <SDL3/SDL_rect.h>
namespace sdlms {

class TriangleGameInstance {
 public:
  static bool RectInsTri(SDL_FRect& r, GameTriangle& t);
  static GameTriangle LoadTri(GameTriangle t, bool flip, SDL_FPoint& pos);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_TRIANGLE_GAME_INSTANCE_H_
