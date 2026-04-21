#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <optional>
#include <string>
class game_obj {
public:
  std::u16string path;
  int64_t z;

  SDL_FPoint pos;
  int8_t flip;

  uint8_t alpha = 255;

  uint32_t ani_index = 0;
  uint32_t ani_time;
  uint8_t ani_count;
};