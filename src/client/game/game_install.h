#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
#include <vector>
class game_install {
public:
  std::u16string id;
  uint32_t time;
  uint8_t index;
};