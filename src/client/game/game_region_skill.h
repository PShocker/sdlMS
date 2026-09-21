#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>

class game_region_skill {
public:
  std::u16string id;
  uint8_t lv;
  SDL_FPoint pos;
  bool flip;

  uint64_t start;
  uint64_t end;
};