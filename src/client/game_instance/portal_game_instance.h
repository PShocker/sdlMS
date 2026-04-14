#pragma once

#include "src/client/game/game_portal.h"
#include <flat_set>
#include <vector>
class portal_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<game_portal> data;
};