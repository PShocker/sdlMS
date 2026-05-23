#pragma once

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <optional>
#include <string>
class game_chatballoon {
public:
  std::u16string text;
  std::u16string path;

  float w = 150.0f;

  SDL_FPoint pos;
  uint64_t destory;
  uint32_t size;

  std::optional<SDL_Color> color;
};