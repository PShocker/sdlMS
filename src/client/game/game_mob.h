#pragma once

#include "SDL3/SDL_rect.h"
#include "game_debuff.h"
#include "game_gauge.h"
#include "src/client/game/game_effect.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <vector>
namespace sdlms {


struct GameMob{
  uint32_t index;
  std::u16string id;

  SDL_FPoint pos;

  std::u16string action;

  uint8_t alpha = 255;

  uint32_t ani_index = 0;
  uint32_t ani_time;
  bool ani_animate = true;

  int64_t hp;
  int64_t mp;

  int64_t max_hp;

  uint8_t page;
  bool flip;

  std::vector<GameEffect> effect;

  std::optional<GameGauge> gauge;

  std::flat_map<GameDebuff::DebuffType, GameDebuff> debuffs;

  // 人物总共打出的伤害，分配经验，伤害统计
  uint64_t attack_val = 0;
};

}  // namespace sdlms
