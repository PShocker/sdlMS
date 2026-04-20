#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>

enum class mob_type {
  Stand,
  Fly,
};

class game_mob {
public:
  std::u16string id;
  int32_t rx0;
  int32_t rx1;
  int32_t fh;

  SDL_FPoint pos;

  std::u16string action;

  uint8_t alpha;

  uint32_t ani_index = 0;
  uint32_t ani_time;

  mob_type type;
};