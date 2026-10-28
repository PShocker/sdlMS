
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>

namespace sdlms {

static void Leidianshu() {
  GameSkill g_skill;
  g_skill.id = u"2201004";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAttackAction(sf, false);
    auto g_r = SkillGameInstance::LoadSkiR(u"2201004");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r);
    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      // Create and send attack payload
      for (int i = 0; i < cm.data.size(); i++) {
        cm.data[i].hits = {60};
        cm.data[i].delay += 120 * i;
      }
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 2201004, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto& Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  Leidianshu();
  return true;
}();

}  // namespace sdlms
