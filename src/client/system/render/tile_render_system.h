#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_TILE_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_TILE_RENDER_SYSTEM_H_

#include "src/client/game/game_tile.h"
namespace sdlms {

class TileRenderSystem {
public:
  static bool Render(GameTile &g_tile);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_TILE_RENDER_SYSTEM_H_
