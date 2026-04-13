#pragma once

#include "src/client/game/game_foothold.h"
#include <cstdint>
#include <vector>
class foothold_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<game_foothold> data;
};