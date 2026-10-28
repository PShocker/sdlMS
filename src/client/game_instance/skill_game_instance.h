#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_SKILL_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_SKILL_GAME_INSTANCE_H_

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

  static fbs::ClientCharacterAttackT CreateAttackPayload(CheckMobs& cm,
                                                         SDL_FPoint pos,
                                                         uint64_t delay);
  static fbs::ClientCharacterAttackT CreateAttackPayload(
      CheckMobs& cm, SDL_FPoint pos, uint64_t delay, uint32_t interval,
      std::string effect, std::string sound);

  static fbs::ClientCharacterSkillT CreateSkillPayload(
      const fbs::ClientCharacterAttackT& attack_payload, int ski_id,
      uint8_t ski_lv);
  static fbs::ClientCharacterSkillT CreateSkillPayload(
      const std::vector<uint64_t>& buff_payload, int ski_id, uint8_t ski_lv);
  static fbs::ClientCharacterSkillT CreateSkillPayload(int ski_id,
                                                       uint8_t ski_lv);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_SKILL_GAME_INSTANCE_H_
