#pragma once

#include "src/client/game/game_backgrnd.h"

namespace sdlms {

class BackgrndRenderSystem {
 public:
  static bool Render(GameBackgrnd& g_backgrnd);
};

}  // namespace sdlms
