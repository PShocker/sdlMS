#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_BALL_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_BALL_RENDER_SYSTEM_H_

#include "src/client/game/game_ball.h"

namespace sdlms {

class BallRenderSystem {
 public:
  static bool Render(GameBall& g_ball);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_BALL_RENDER_SYSTEM_H_
