#pragma once

#include "SDL3/SDL_rect.h"
#include "server_equip.h"
#include <cstdint>
#include <optional>
class server_drop {
public:
  uint32_t id;
  uint32_t num;

  SDL_FPoint pos;
  int32_t rotate;
  uint64_t destory;

  bool fly;

  uint64_t picker;

  float hspeed = 0;
  float vspeed = 0;

  std::optional<server_equip> equip;
};