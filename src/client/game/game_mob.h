#pragma once

#include "SDL3/SDL_rect.h"
#include <string>
class game_mob {
public:
  uint32_t id;
  int32_t rx0;
  int32_t rx1;
  int32_t fh;

  SDL_FPoint pos;

  std::u16string action;

  uint32_t ani_index = 0;
  uint32_t ani_time;
};