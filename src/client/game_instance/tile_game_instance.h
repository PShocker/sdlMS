#pragma once
#include "src/client/game/game_tile.h"
#include <flat_map>
#include <vector>
class tile_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::array<std::flat_multimap<int64_t, game_tile>, 8> data;
};