#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class game_portal {
public:
  int32_t tm = 0;
  int32_t pt = 0;
  std::u16string pn; // portal_name
  std::u16string tn; // portal_goal

  std::u16string path;
  std::u16string path2;

  SDL_FPoint pos;
  int64_t z;
  // 传送门
  uint32_t ani_index = 0;
  uint32_t ani_time;
};