#pragma once

#include "SDL3/SDL_rect.h"
#include "server_debuff.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

struct MobHit {
  uint64_t hit_id; // client_id
  uint64_t hit_time;
  bool hit_push;
  int64_t hit_num;
  bool left;
};

class ServerMob {
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

  float hspeed;
  float hspeed_max;
  float vspeed;

  float hforce = 0;
  uint64_t duration;

  uint64_t hate_id;

  std::flat_map<uint64_t, MobHit> hits;

  std::flat_map<GameDebuff::DebuffType, ServerDebuff> debuffs;

  bool create = false; // 是否是道具召唤的

  enum class MobType {
    kStand,
    kSwim,
    kFly,
  };
  MobType type;
};