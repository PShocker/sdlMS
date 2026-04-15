#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class game_nametag {
public:
  std::u8string text;
  std::u16string path;

  SDL_FPoint pos;
  uint32_t size;
};