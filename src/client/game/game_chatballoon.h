#pragma once

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class game_chatballoon {
public:
  std::u8string text;
  std::u16string path;

  SDL_FPoint pos;
  SDL_Color color;
  uint64_t destory;
};