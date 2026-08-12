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

  uint8_t state;
  uint64_t hit_cd;

  uint64_t revive = 0; // 刷新时间
};