#include "src/client/game_skill/mp_eater.h"

#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"

#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <random>
#include <string>

namespace sdlms {

namespace {

int SkillInt(const std::u16string& id, int lv, const char16_t* key,
             int32_t def) {
  return SkillGameInstance::LoadSkiLevelInt(
      id, static_cast<uint8_t>(std::max(1, lv)), key, def);
}

void SendMpEaterEffect(uint32_t skill_num, int lv) {
  fbs::ClientCharacterAttackT cat;
  auto ckt = SkillGameInstance::CreateSkillPayload(cat, skill_num, lv);
  ServerCharacterInstance::HandleSki(ckt.ski_id, lv, ckt.payload, 0);
  ClientRequest::SendToHost(ckt);
}

void TryMpEater(CheckMobs& cm, int ski_lv, const std::u16string& skill_id,
                uint32_t skill_num) {
  static std::flat_map<uint32_t, uint64_t> next;
  const uint64_t now = Window::dt_now_;
  if (now < next[skill_num] || cm.data.empty() || ski_lv <= 0) {
    return;
  }
  const int prop = SkillInt(skill_id, ski_lv, u"prop", 0);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) >= prop) {
    return;
  }
  auto& target = cm.data[0];
  auto it = MobGameInstance::data_.find(target.mob.index);
  if (it == MobGameInstance::data_.end() || it->second.mob.mp <= 0) {
    return;
  }
  const int x = std::max(0, SkillInt(skill_id, ski_lv, u"x", 0));
  const int64_t drain =
      std::max<int64_t>(1, it->second.mob.mp * x / 100);
  it->second.mob.mp = std::max<int64_t>(0, it->second.mob.mp - drain);
  target.mob.mp = it->second.mob.mp;
  CharacterStatGameInstance::SetMp(std::min(
      CharacterStatGameInstance::GetMpMax(),
      CharacterStatGameInstance::GetMp() + static_cast<int32_t>(drain)));
  next[skill_num] = now + 5000;

  SendMpEaterEffect(skill_num, std::max(1, ski_lv));
}

}  // namespace

void TriggerMpEater(CheckMobs& cm) {
  const auto& sf = CharacterGameInstance::self_;
  if (sf.job == u"210") {
    const int lv = JobSkillGameInstance::LoadSkiLevel(u"2100000");
    if (lv > 0) {
      TryMpEater(cm, lv, u"2100000", 2100000);
    }
  } else if (sf.job == u"220") {
    const int lv = JobSkillGameInstance::LoadSkiLevel(u"2200000");
    if (lv > 0) {
      TryMpEater(cm, lv, u"2200000", 2200000);
    }
  } else if (sf.job == u"230") {
    const int lv = JobSkillGameInstance::LoadSkiLevel(u"2300000");
    if (lv > 0) {
      TryMpEater(cm, lv, u"2300000", 2300000);
    }
  }
}

}  // namespace sdlms
