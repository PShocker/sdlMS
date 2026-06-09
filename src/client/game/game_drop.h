#pragma once

#include "SDL3/SDL_rect.h"
#include "game_equip.h"
#include "game_pet.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_pet.h"
#include <any>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>
class game_drop {
public:
  uint64_t random_id;

  std::variant<game_equip, game_item> data;

  SDL_FPoint pos;
  SDL_FPoint goal;

  int32_t rotate;
  uint64_t destory;

  float hspeed = 0;
  float vspeed = 0;

  uint8_t page;

  enum drop_type {
    fly,
    land,
    pick,
  };

  drop_type type = fly;

  std::variant<game_character *, game_pet *> picker;
};