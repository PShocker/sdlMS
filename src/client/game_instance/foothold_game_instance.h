#pragma once

#include "src/client/game/game_foothold.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <vector>
namespace sdlms {

class FootholdGameInstance {
public:
  static std::flat_map<int32_t, GameFoothold> Load(uint32_t map_id);
  static std::flat_map<int32_t, GameFoothold> Load(wz::Node *node);
  static inline std::flat_map<int32_t, GameFoothold> data_;
};

}  // namespace sdlms
