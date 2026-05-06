#pragma once

#include "SDL3/SDL_rect.h"
#include "game_equip.h"
#include "game_pet.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_pet.h"
#include <any>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>
class game_drop {
public:
  uint32_t id;
  uint32_t num;

  uint8_t type;

  SDL_FPoint pos;
  int32_t rotate;
  uint64_t destory;

  bool fly;

  float hspeed = 0;
  float vspeed = 0;

  std::variant<game_character *, game_pet *> picker;

  std::optional<game_equip> equip;
};