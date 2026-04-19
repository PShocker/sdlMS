#pragma once

#include "src/client/game/game_drop.h"
#include <flat_map>
#include <vector>
class drop_game_instance {
public:
  static void load_item(uint32_t item_id);
  static inline std::array<std::vector<game_drop>, 8> data;
};