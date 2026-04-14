#pragma once

#include "src/client/game/game_obj.h"
#include <array>
#include <cstdint>
#include <flat_map>
class obj_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::array<std::flat_multimap<int64_t, game_obj>, 8> data;
};