#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
#include <variant>
class game_portal {
public:
  int32_t tm = 0;
  int32_t pt = 0;
  std::u16string pn;
  std::variant<std::u16string, SDL_FPoint> tn;

  // 传送门
  uint32_t ani_index = 0;
  uint32_t ani_time;
};