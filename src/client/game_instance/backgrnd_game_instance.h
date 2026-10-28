#pragma once

#include "src/client/game/game_backgrnd.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
class BackgrndGameInstance {
public:
  static void load(wz::Node *image);
  static void load(uint32_t map_id);
  static inline std::flat_map<int32_t, GameBackgrnd> front;
  static inline std::flat_map<int32_t, GameBackgrnd> back;
};