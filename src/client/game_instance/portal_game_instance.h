#pragma once

#include "src/client/game/game_portal.h"
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>
class portal_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::flat_map<std::u16string, game_portal> data;
};