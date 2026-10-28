#pragma once

#include "SDL3/SDL_rect.h"
#include "game_region_skill.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include "src/server/server/server_region_skill.h"
#include <cstdint>
#include <functional>
#include <string>
struct GameSkill{
  std::u16string id;
  std::function<void(uint64_t, int)> use;
  std::function<void()> frame;
  std::function<void(int)> passive;
  std::function<void()> end;
  std::function<bool(SDL_FPoint, GameEffect *, bool)> effect;
  std::function<void(GameSkill *, std::u16string)>
      action;                                        // 斗气集中,终极攻击,影分身
  std::function<uint64_t(GameMob *, uint64_t)> hit; // 魔法盾，金钱护盾
  std::function<void(GameCharacter *, int)> state;

  std::function<void(GameRegionSkill *)> client_region;
  std::function<void(ServerRegionSkill *)> server_region;

  bool fall = false;
  bool climb = false;

  uint64_t destroy;
  uint64_t duration;
  uint64_t cd;
  uint64_t start;
  uint8_t lv;
};