#pragma once
#include "src/client/game/game_reactor.h"
#include <cstdint>
#include <vector>
class reactor_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::array<std::vector<game_reactor>, 8> data;

private:
  static uint8_t load_reactor_layer(SDL_FPoint pos);
};