#pragma once
#include "src/client/game/game_tooltip.h"
#include <flat_map>
#include <vector>
class tooltip_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<game_tooltip> data;
};