#ifndef SDLMS_SRC_CLIENT_GAME_GAME_TRIANGLE_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_TRIANGLE_H_

#include <SDL3/SDL_rect.h>
#include <array>
namespace sdlms {

class GameTriangle {
public:
  std::array<SDL_FPoint, 3> point;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_TRIANGLE_H_
