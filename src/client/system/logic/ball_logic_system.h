#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
class BallLogicSystem {
private:
  static SDL_FPoint move(SDL_FPoint p1,SDL_FPoint p2,int speed);
  static void run_animate(GameBall &b);
  static bool run_move(GameBall &b);

public:
  static bool run();
};