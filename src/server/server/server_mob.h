#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_mob.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

struct mob_hit {
  uint64_t hit_id; // client_id
  uint64_t hit_time;
  int32_t hit_duration;
  int64_t hit_num;
  bool left;
};

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

  uint64_t hate_id;

  std::flat_map<uint64_t, mob_hit> hits;

  enum class mob_type {
    stand,
    swim,
    fly,
  };
  mob_type type;
};