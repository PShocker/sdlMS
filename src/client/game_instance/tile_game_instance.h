#pragma once
#include "src/client/game/game_tile.h"
#include "wz/Node.h"
#include <flat_map>
#include <vector>
namespace sdlms {

class TileGameInstance {
public:
  static void Load(wz::Node *image);
  static void Load(uint32_t map_id);
  static inline std::array<std::flat_multimap<int64_t, GameTile>, 8> data_;
};

}  // namespace sdlms
