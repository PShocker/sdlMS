#pragma once
#include "src/client/game/game_reactor.h"
#include <vector>
class reactor_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::array<std::vector<game_reactor>, 8> data;
};