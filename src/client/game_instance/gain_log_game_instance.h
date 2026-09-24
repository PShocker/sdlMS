#pragma once

#include "src/client/game/game_gain_log.h"
#include <cstdint>
#include <string>
#include <vector>
class gain_log_game_instance {
public:
  static inline std::vector<game_gain_log> data;

  static void add(const std::u16string &id, int num, gain_enum type);
};