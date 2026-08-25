#pragma once

#include "SDL3/SDL_rect.h"
#include "wz/Node.h"
#include <vector>
class clock_game_instance {
public:
  static void load(wz::Node *image);
  static void load(uint32_t map_id);
  static inline std::vector<SDL_FRect> data;
};