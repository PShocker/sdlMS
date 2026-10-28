#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
struct GameTooltip{
  std::u16string path;
  SDL_FRect r;
};