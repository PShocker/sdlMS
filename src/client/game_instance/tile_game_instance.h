#pragma once
#include "src/client/game/game_tile.h"
#include "wz/Node.h"
#include <flat_map>
#include <vector>
class TileGameInstance {
public:
  static void load(wz::Node *image);
  static void load(uint32_t map_id);
  static inline std::array<std::flat_multimap<int64_t, GameTile>, 8> data;
};