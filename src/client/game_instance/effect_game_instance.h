#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>

using namespace fbs;

class effect_game_instance {
public:
  static inline std::flat_map<uint64_t, std::vector<game_effect>>
      effect; // character

  static inline std::flat_map<int32_t, std::vector<game_effect>> effect2; // mob

  static inline std::array<std::vector<game_effect>, 8> data;

  static void load_character_attack(
      const std::vector<std::unique_ptr<fbs::CharacterAttackT>> &v,
      game_character &g_character);

  static void load_character_skill(uint32_t ski_id,
      const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
      game_character &g_character);
  static void reset();
};