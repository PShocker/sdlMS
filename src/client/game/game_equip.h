#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
#include <vector>
class game_equip {
public:
  std::u16string id;

  std::vector<uint32_t> scroll;
  uint8_t upgrade;
};