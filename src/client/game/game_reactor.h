#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class game_reactor {
public:
  std::u16string id;
  SDL_FPoint pos;

  uint8_t reactor_index;
  uint32_t reactor_time;
  bool f;

  uint32_t ani_index = 0;
  uint32_t ani_time;
};