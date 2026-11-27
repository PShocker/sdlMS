#pragma once

#include <SDL3/SDL_rect.h>

#include <any>
#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

struct DamageData {
  uint64_t num;
  // kPurple 追加在末尾：人物受伤专用（095 玩家受伤数字为紫色，
  // 资源 BasicEff.img/NoViolet0）。kWhite 历史上也映射到同一套紫色资源。
  enum class DamageColor {
    kRed,
    kBlue,
    kWhite,
    kCrit,
    kFinalAttack,
    kMiss,
    kPurple,
  };
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
