#pragma once

#include "src/client/game/game_ladderrope.h"
#include <vector>
class ladderrope_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<game_ladderrope> data;
};