#pragma once

#include "src/client/game/game_drop.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
#include <flat_map>
#include <vector>

using namespace fbs;

class drop_game_instance {
public:
  static void load_drop(const DropT &dt);

  static void pick_drop(uint64_t client_id, ServerCharacterDropT &r);

  static inline std::flat_map<uint64_t, game_drop> data;
};