#pragma once

#include "src/client/game/game_reactor.h"

#include <array>
#include <cstdint>
#include <vector>

namespace sdlms {

class ReactorGameInstance {
 public:
  static void Load(uint32_t map_id);
  static inline std::array<std::vector<GameReactor>, 8> data_;

 private:
  static uint8_t LoadReactorLayer(SDL_FPoint pos);
};

}  // namespace sdlms
