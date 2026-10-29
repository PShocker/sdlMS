#pragma once

#include "src/client/game/game_drop.h"

namespace sdlms {

class DropRenderSystem {
 public:
  static bool Render(GameDrop& g_drop);
};

}  // namespace sdlms
