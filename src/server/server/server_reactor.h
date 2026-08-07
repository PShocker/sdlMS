#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
#include <vector>

class server_reactor {
public:
  std::u16string id;
  uint32_t index;
  SDL_FPoint pos;
  uint8_t act;
};