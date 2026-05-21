#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include <vector>

class ball_game_instance {
public:
  static inline std::vector<game_ball> data;
  static void reset();
  static void load();
};