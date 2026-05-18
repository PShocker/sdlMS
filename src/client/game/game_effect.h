#pragma once

#include "SDL3/SDL_rect.h"
#include <any>
#include <cstdint>
#include <optional>
#include <string>
class game_effect {
public:
  std::u16string ski_id;

  uint32_t index;
  uint32_t time;

  uint64_t delay;

  enum class effect_type {
    normal,
    custom,
  };
  effect_type type;

  std::optional<SDL_FPoint> pos;
  // custom
  std::any data;
};