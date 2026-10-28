#pragma once

#include "src/client/game/game_backgrnd.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
namespace sdlms {

class BackgrndGameInstance {
public:
  static void Load(wz::Node *image);
  static void Load(uint32_t map_id);
  static inline std::flat_map<int32_t, GameBackgrnd> front_;
  static inline std::flat_map<int32_t, GameBackgrnd> back_;
};

}  // namespace sdlms
