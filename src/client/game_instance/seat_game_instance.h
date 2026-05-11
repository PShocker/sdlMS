#pragma once
#include "src/client/game/game_seat.h"
#include "SDL3/SDL_rect.h"
#include <vector>
class seat_game_instance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<game_seat> data;
};