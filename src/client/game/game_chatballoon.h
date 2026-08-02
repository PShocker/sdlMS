#pragma once

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <optional>
#include <string>
class game_chatballoon {
public:
  std::u16string text;
  std::u16string path = u"0";

  float w = 150.0f;

  SDL_FPoint pos = {0, 0};
  uint64_t destroy‌;
  uint32_t size = 14;

  std::optional<SDL_Color> color;
};