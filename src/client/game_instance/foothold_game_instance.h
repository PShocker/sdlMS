#pragma once

#include "src/client/game/game_foothold.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <vector>
class FootholdGameInstance {
public:
  static std::flat_map<int32_t, GameFoothold> load(uint32_t map_id);
  static std::flat_map<int32_t, GameFoothold> load(wz::Node *node);
  static inline std::flat_map<int32_t, GameFoothold> data;
};