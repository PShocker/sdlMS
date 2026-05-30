#pragma once

#include "SDL3/SDL_rect.h"
#include <any>
#include <cstdint>
#include <optional>
#include <string>

class game_effect {
public:
  std::u16string id;

  uint32_t index = 0;
  uint32_t time = 0;
  uint8_t alpha = 255;

  uint64_t delay;

  enum class effect_type {
    afterimage,
    damage,
    skill_use,
    skill_hit,
    custom,
  };
  effect_type type;

  std::optional<SDL_FPoint> pos;
  std::optional<bool> z;
  std::optional<bool> flip;
  std::any data;
};