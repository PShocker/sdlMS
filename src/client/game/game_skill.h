#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include "src/client/game/game_region_skill.h"
#include "src/server/server/server_region_skill.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <functional>
#include <string>

namespace sdlms {

// 主动技能持续期间提供的属性加成 (由 Skill.wz 的 level 节点解析,
// 个别技能可用 GameSkill::buff_stat 覆盖/补充)
struct SkillBuffStat {
  int32_t hp = 0;
  int32_t mp = 0;
  int32_t pad = 0;    // 物理攻击
  int32_t mad = 0;    // 魔法攻击
  int32_t acc = 0;    // 命中
  int32_t eva = 0;    // 回避
  int32_t pdd = 0;       // 物理防御
  int32_t pdd_rate = 0;  // 物理防御百分比 (Iron Body)
  int32_t mdd = 0;       // 魔法防御
  int32_t speed = 0;  // 移动速度
  int32_t jump = 0;   // 跳跃力
};

struct GameSkill {
  std::u16string id;
  std::function<void(uint64_t, int)> use;
  // 释放前的额外条件（道具、状态等）；返回 false 时不扣 MP、不进冷却。
  std::function<bool(int)> can_use;
  // 返回 true 时本次释放不扣 MP（例如 Magic Guard 关闭）。
  std::function<bool()> skip_mp;
  std::function<void()> frame;
  std::function<void(int)> passive;
  std::function<void()> end;
  std::function<bool(SDL_FPoint, GameEffect*, bool)> effect;
  std::function<void(GameSkill*, std::u16string)>
      action;                                       // 斗气集中,终极攻击,影分身
  std::function<uint64_t(GameMob*, uint64_t)> hit;  // 魔法盾，金钱护盾
  std::function<void(GameCharacter*, int)> state;
  // 覆盖/补充 buff 属性 (参数: 技能等级, 待填的属性)
  std::function<void(int, SkillBuffStat&)> buff_stat;

  std::function<void(GameRegionSkill*)> client_region;
  std::function<void(ServerRegionSkill*)> server_region;

  bool fall = false;
  bool climb = false;

  uint64_t destroy = 0;
  uint64_t duration = 0;
  uint64_t cd = 0;
  uint64_t start = 0;
  uint8_t lv = 0;
};

}  // namespace sdlms
