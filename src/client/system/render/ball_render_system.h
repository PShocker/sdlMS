#pragma once

#include "src/client/game/game_ball.h"
namespace sdlms {

class BallRenderSystem {
public:
  static bool Render(GameBall &g_ball);
};

}  // namespace sdlms
