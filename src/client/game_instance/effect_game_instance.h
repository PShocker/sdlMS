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
  static inline std::array<std::vector<game_effect>, 8> data;
  static void reset();
};