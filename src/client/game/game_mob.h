#pragma once

#include "SDL3/SDL_rect.h"
#include "game_gauge.h"
#include "src/client/game/game_effect.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class game_mob {
public:
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

  std::vector<game_effect> effect;

  std::optional<game_gauge> gauge;
};