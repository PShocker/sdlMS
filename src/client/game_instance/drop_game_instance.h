#pragma once

#include "src/client/game/game_drop.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
#include <flat_map>
#include <vector>
namespace sdlms {


using namespace fbs;

class DropGameInstance {
public:
  static void Reset();

  static inline std::flat_map<uint64_t, GameDrop> data_;
};

}  // namespace sdlms
