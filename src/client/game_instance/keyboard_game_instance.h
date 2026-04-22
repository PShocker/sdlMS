#pragma once

#include "src/client/game/game_input.h"
#include <cstdint>
#include <flat_map>
class keyboard_game_instance {
private:
  static void reset();
public:
  static void load();
  static inline std::flat_map<uint8_t, game_input> data;
};