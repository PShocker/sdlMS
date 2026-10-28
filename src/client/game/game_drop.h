#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <memory>
#include <optional>


struct GameDropPick {
  uint64_t client_id;
  std::optional<uint8_t> pet_index;
};

class GameDrop {
public:
  uint64_t random_id;

  std::polymorphic<GameItem> data;

  SDL_FPoint pos;
  SDL_FPoint goal;

  int32_t rotate;
  uint8_t alpha = 255;

  float hspeed = 0;
  float vspeed = 0;

  uint8_t page;

  enum class DropEnum {
    fly,
    land,
    pick,
    fade,
  };

  DropEnum type = DropEnum::fly;

  uint64_t pick_time;
  std::optional<GameDropPick> picker;
};