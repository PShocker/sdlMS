#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
class ball_logic_system {
private:
  static SDL_FPoint move(SDL_FPoint p1,SDL_FPoint p2,int speed);
  static void run_animate(game_ball &b);
  static bool run_move(game_ball &b);

public:
  static bool run();
};