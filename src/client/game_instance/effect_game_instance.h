#pragma once

#include "src/client/game/game_effect.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <string>

class effect_game_instance {
public:
  static inline std::flat_map<uint64_t, game_effect> character_before_effect;
  static inline std::flat_map<uint64_t, game_effect> character_after_effect;

  static inline std::flat_map<int32_t, game_effect> mob_before_effect;
  static inline std::flat_map<int32_t, game_effect> mob_after_effect;

  static inline std::array<std::vector<game_effect>, 8> data;
};