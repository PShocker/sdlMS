#pragma once

#include "SDL3/SDL_rect.h"
#include <any>
#include <cstdint>
#include <optional>
#include <string>

struct damage_data {
  uint64_t num;
  enum t { r, b, v };
  t type;
};

class game_effect {
public:
  std::u16string id;

  uint32_t index = 0;
  uint32_t time = 0;
  uint8_t alpha = 255;

  uint64_t delay;
  uint8_t lvl = 0;

  enum class effect_type {
    effect,
    damage,
    skill_use,
    skill_hit,
    skill_custom,
  };
  effect_type type;

  std::optional<SDL_FPoint> pos;
  std::optional<bool> z;
  std::optional<bool> flip;
  std::any data;
};