#pragma once

#include "src/client/game/game_npc.h"
#include <flat_map>
#include <vector>
class npc_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::array<std::vector<game_npc>, 8> data;
};