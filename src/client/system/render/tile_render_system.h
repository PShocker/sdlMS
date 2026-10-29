#pragma once

#include "src/client/game/game_tile.h"

namespace sdlms {

class TileRenderSystem {
 public:
  static bool Render(GameTile& g_tile);
};

}  // namespace sdlms
