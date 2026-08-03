#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <memory>
#include <optional>


struct game_drop_pick {
  uint64_t client_id;
  std::optional<uint8_t> pet_index;
};

class game_drop {
public:
  uint64_t random_id;

  std::polymorphic<game_item> data;

  SDL_FPoint pos;
  SDL_FPoint goal;

  int32_t rotate;
  uint8_t alpha = 255;

  float hspeed = 0;
  float vspeed = 0;

  uint8_t page;

  enum class drop_enum {
    fly,
    land,
    pick,
    fade,
  };

  drop_enum type = drop_enum::fly;

  uint64_t pick_time;
  std::optional<game_drop_pick> picker;
};