#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
class game_ladderrope {
public:
  uint8_t l;
  uint8_t uf;
  int32_t x;
  int32_t y1;
  int32_t y2;

  int32_t t;
  int32_t b;

  uint8_t page;
  uint8_t piece;
};