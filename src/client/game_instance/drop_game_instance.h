#pragma once

#include "src/client/game/game_drop.h"
#include "src/common/flatbuffers/server.h"
#include <flat_map>
#include <vector>

using namespace fbs;

class drop_game_instance {
public:
  static void load_item(uint32_t item_id);
  static void load_character_drop(const ServerCharacterDropT& d);
  static inline std::array<std::vector<game_drop>, 8> data;
};