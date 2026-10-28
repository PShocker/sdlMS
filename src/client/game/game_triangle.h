#pragma once

#include "SDL3/SDL_rect.h"
#include <array>
namespace sdlms {

class GameTriangle {
public:
  std::array<SDL_FPoint, 3> point;
};

}  // namespace sdlms
