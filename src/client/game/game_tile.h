#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class game_tile {
public:
  SDL_FPoint pos;
  int64_t z;

  std::u16string path;
  uint8_t alpha = 255;
};