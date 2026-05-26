#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class game_tomb {
public:
  std::u16string ani_type;
  uint8_t ani_index;
  uint32_t ani_time;

  SDL_FPoint pos;
  SDL_FPoint b;

  float rotation = 0;
};