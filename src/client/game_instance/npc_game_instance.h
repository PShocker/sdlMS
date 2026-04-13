#pragma once

#include "src/client/game/game_npc.h"
#include <vector>
class npc_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<game_npc> data;
};