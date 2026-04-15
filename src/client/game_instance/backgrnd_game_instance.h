#pragma once

#include "src/client/game/game_backgrnd.h"
#include <cstdint>
#include <flat_map>
class backgrnd_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::flat_map<int32_t, game_backgrnd> front;
  static inline std::flat_map<int32_t, game_backgrnd> back;
};