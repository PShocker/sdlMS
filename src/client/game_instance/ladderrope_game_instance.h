#pragma once

#include "src/client/game/game_mob.h"
#include <vector>
class mob_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<game_mob> data;
};