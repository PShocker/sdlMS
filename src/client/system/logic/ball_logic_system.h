#pragma once

#include "src/client/game/game_ball.h"

#include <SDL3/SDL_rect.h>

namespace sdlms {

class BallLogicSystem {
 private:
  static SDL_FPoint Move(SDL_FPoint p1, SDL_FPoint p2, int speed);
  static void RunAnimate(GameBall& b);
  static bool RunMove(GameBall& b);

 public:
  static bool Run();
};

}  // namespace sdlms
