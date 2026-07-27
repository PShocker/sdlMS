#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include <cstdint>
#include <functional>
#include <string>
class game_skill {
public:
  std::u16string id;
  std::function<void(int)> use;
  std::function<void()> frame;
  std::function<void()> passive;
  std::function<void()> end;
  std::function<void(SDL_FPoint, game_effect, bool)> effect;
  std::function<void(game_mob, uint64_t)> hit;

  bool fall = false;
  bool climb = false;

  uint64_t destory;
  uint64_t duration;
  uint64_t cd;
  uint64_t start;
  uint8_t lv;
};