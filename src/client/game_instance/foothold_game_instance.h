#pragma once

#include "src/client/game/game_foothold.h"
#include <cstdint>
#include <flat_map>
class foothold_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::flat_map<int32_t, game_foothold> data;
};