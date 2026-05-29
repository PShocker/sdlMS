#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <optional>
#include <string>

class game_buff {
public:
  std::u16string id;
  uint8_t level;
};