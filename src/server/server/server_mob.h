#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_mob.h"
#include <cstdint>
#include <optional>
#include <string>
class server_mob {
public:
  uint32_t index;
  std::u16string id;

  SDL_FPoint pos;

  std::u16string action;

  int64_t hp;
  int64_t mp;

  uint8_t page;
  bool flip;

  uint64_t revive; // 复活时间

  int32_t rx0;
  int32_t rx1;
  int32_t fh;

  float hspeed = 100;
  float vspeed;

  float hforce = 1400;
  uint64_t duration;

  enum class mob_type {
    stand,
    swim,
    fly,
  };
  mob_type type;
};