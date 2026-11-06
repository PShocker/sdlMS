#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_skill.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/client.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

namespace sdlms {

class SkillGameInstance {
 public:
  static inline std::vector<GameSkill> ski_;  // buff
  static auto& Skis() {
    static std::flat_map<std::u16string, GameSkill> instance;
    return instance;
  }

  static wz::Node* LoadSkiNode(uint32_t id);
  static wz::Node* LoadSkiNode(const std::string& id);
  static wz::Node* LoadSkiNode(const std::u16string& id);
  static wz::Node* LoadSkiLevelNode(const std::u16string& id, uint8_t l);
  // 从 level 节点解析这个技能 buff 提供哪些属性
  static SkillBuffStat LoadSkiBuffStat(const std::u16string& id, uint8_t lv);
  // 读 level 节点里的 int 字段 (字符串数字也解析), 取不到返回 def
  static int32_t LoadSkiLevelInt(const std::u16string& id, uint8_t lv,
                                 const char16_t* key, int32_t def);

  static uint64_t LoadSkiTime(GameCharacter& g_character);
  static SDL_FRect LoadR(SDL_FPoint lt, SDL_FPoint rb, SDL_FPoint pos,
                         bool flip);

  struct SkillName {
    std::u16string name;
    std::u16string desc;
    std::vector<std::u16string> level;
  };
  static SkillName LoadSkiName(const std::u16string& id);
  static int LoadSkiMaxLvl(const std::u16string& id);
  static bool LoadSkiActive(const std::u16string& id);

  static SDL_FRect LoadSkiR(const std::u16string& id);
  static SDL_FRect LoadSkiR(const std::u16string& id, SDL_FPoint pos,
                            bool flip);
  static int LoadSkiBallNum(const std::u16string& id, uint8_t lv);
  static int LoadSkiMobCount(const std::u16string& id, uint8_t lv);
  static int LoadSkiAtkCount(const std::u16string& id, uint8_t lv);
  // 主动技能 MP 消耗（Skill.wz level/<lv>/mpCon）。
  static int LoadSkiMpCost(const std::u16string& id, uint8_t lv);
  // 主动技能冷却（秒，Skill.wz level/<lv>/cooltime）。
  static int LoadSkiCooldown(const std::u16string& id, uint8_t lv);

  static fbs::ClientCharacterAttackT CreateAttackPayload(CheckMobs& cm,
                                                         SDL_FPoint pos,
                                                         uint64_t delay);
  static fbs::ClientCharacterAttackT CreateAttackPayload(
      CheckMobs& cm, SDL_FPoint pos, uint64_t delay, uint32_t interval,
      std::string effect, std::string sound, float knockback = 0.0f);

  static fbs::ClientCharacterSkillT CreateSkillPayload(
      const fbs::ClientCharacterAttackT& attack_payload, int ski_id,
      uint8_t ski_lv);
  static fbs::ClientCharacterSkillT CreateSkillPayload(
      const std::vector<uint64_t>& buff_payload, int ski_id, uint8_t ski_lv);
  static fbs::ClientCharacterSkillT CreateSkillPayload(int ski_id,
                                                       uint8_t ski_lv);
};

}  // namespace sdlms
