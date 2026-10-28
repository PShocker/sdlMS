#pragma once

#include "src/client/game/game_ladderrope.h"
#include <flat_map>
#include <vector>
class LadderropeGameInstance {
public:
  static void load(uint32_t map_id);
  static inline std::flat_map<int32_t, GameLadderrope> data;
};