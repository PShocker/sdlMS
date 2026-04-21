#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
#include <vector>
class server_equip {
public:
  uint32_t id;

  std::vector<uint32_t> scroll;
  uint8_t upgrade;
};