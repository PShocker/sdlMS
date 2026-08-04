#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <optional>
#include <string>

class game_chair {
public:
  std::u16string id;
  uint8_t ani_index = 0;
  uint32_t ani_time = 0;
};