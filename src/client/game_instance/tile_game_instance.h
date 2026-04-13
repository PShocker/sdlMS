#pragma once
#include "src/client/game/game_tile.h"
#include <vector>
class tile_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<game_tile> data;
};