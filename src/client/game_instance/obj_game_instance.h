#pragma once

#include "src/client/game/game_obj.h"
#include <vector>
class obj_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<game_obj> data;
};