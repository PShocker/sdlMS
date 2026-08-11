#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class game_reactor {
public:
  std::u16string id;
  uint8_t index;

  SDL_FPoint pos;

  bool f;

  uint8_t state;
  uint32_t ani_index = 0;
  uint32_t ani_time;
  bool hit = false;
};