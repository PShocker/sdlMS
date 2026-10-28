#pragma once

#include "SDL3/SDL_rect.h"
#include <array>
class GameTriangle {
public:
  std::array<SDL_FPoint, 3> point;
};