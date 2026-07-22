#pragma once

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
  std::function<void(game_mob, uint64_t)> hit;

  uint64_t destory;
  uint64_t duration;
  uint64_t cd;
  uint8_t lv;
};