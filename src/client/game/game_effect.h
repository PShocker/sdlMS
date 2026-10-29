#pragma once

#include <SDL3/SDL_rect.h>

#include <any>
#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

struct DamageData {
  uint64_t num;
  enum class DamageColor { kRed, kBlue, kWhite };
  DamageColor type;
};

struct GameEffect {
  std::u16string id;

  uint32_t index = 0;
  uint32_t time = 0;
  uint8_t alpha = 255;

  uint64_t delay;
  uint8_t lv = 0;

  enum class EffectType {
    kEffect,
    kDamage,
    kSkillUse,
    kSkillAffected,
    kSkillHit,
    kSkillCustom,
    kSkillRegion,
  };

  enum class EffectSubType : uint8_t {
    kCharacter,
    kEffect,
    kItem,
    kSkill,
  };

  EffectType type;

  std::optional<SDL_FPoint> pos;
  std::optional<bool> z;
  std::optional<bool> flip;
  std::optional<uint8_t> order;
  std::any data;
};

}  // namespace sdlms
