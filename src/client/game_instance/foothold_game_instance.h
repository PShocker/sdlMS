#pragma once

#include "src/client/game/game_foothold.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <vector>
class foothold_game_instance {
public:
  static std::flat_map<int32_t, game_foothold> load(uint32_t map_id);
  static std::flat_map<int32_t, game_foothold> load(wz::Node *node);
  static inline std::flat_map<int32_t, game_foothold> data;
};