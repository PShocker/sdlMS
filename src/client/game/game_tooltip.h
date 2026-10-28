#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
namespace sdlms {

struct GameTooltip{
  std::u16string path;
  SDL_FRect r;
};

}  // namespace sdlms
