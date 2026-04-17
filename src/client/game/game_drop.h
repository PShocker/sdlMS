#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include <any>
#include <cstdint>
#include <vector>
class game_drop {
public:
  uint32_t id;
  uint32_t num;

  uint8_t type;
  std::any data;

  SDL_FPoint pos;
  int32_t rotate;
  uint64_t destory;

  bool fly;

  game_character *picker;

  std::vector<uint32_t> scroll;
  uint8_t upgrade;
};