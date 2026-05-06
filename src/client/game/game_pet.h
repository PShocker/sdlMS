#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
class game_pet {
public:
  game_character *g_character;

  SDL_FPoint pos;
  int64_t z;
  // 传送门
  uint32_t ani_index = 0;
  uint32_t ani_time;
};