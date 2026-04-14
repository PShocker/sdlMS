#pragma once

#include "src/client/game/game_mob.h"
#include <vector>
class mob_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::array<std::vector<game_mob>, 8> data;
};