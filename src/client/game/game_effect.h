#pragma once

#include "SDL3/SDL_rect.h"
#include <any>
#include <cstdint>
#include <optional>
#include <string>

struct DamageData {
  uint64_t num;
  enum t { r, b, v };
  t type;
};

class GameEffect {
public:
  std::u16string id;

  uint32_t index = 0;
  uint32_t time = 0;
  uint8_t alpha = 255;

  uint64_t delay;
  uint8_t lv = 0;

  enum class EffectType {
    effect,
    damage,
    skill_use,
    skill_affected,
    skill_hit,
    skill_custom,
    skill_region,
  };

  enum EffectSubType : uint8_t {
    character,
    effect,
    item,
    skill,
  };

  EffectType type;

  std::optional<SDL_FPoint> pos;
  std::optional<bool> z;
  std::optional<bool> flip;
  std::optional<uint8_t> order;
  std::any data;
};