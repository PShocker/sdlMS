#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
struct GameTile{
  SDL_FPoint pos;
  int64_t z;

  std::u16string path;
  uint8_t alpha = 255;
};