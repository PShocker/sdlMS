#pragma once

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class game_nametag {
public:
  std::u16string text;
  std::u16string path;

  SDL_Color color;
  SDL_FPoint pos;
  uint32_t size;
};