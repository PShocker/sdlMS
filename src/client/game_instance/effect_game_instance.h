#pragma once

#include "src/client/game/game_effect.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>

class effect_game_instance {
public:
  static inline std::flat_map<uint64_t, std::vector<game_effect>> c_effect;

  static inline std::flat_map<int32_t, std::vector<game_effect>> m_effect;

  static inline std::array<std::vector<game_effect>, 8> data;

  static void reset();
};