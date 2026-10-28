
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>

static void leidianshu() {
  GameSkill g_skill;
  g_skill.id = u"2201004";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_attack_action(sf, false);
    auto g_r = SkillGameInstance::load_ski_r(u"2201004");
    auto cm = CharacterLogicSystem::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::load_ski_time(sf);
      // Create and send attack payload
      for (int i = 0; i < cm.data.size(); i++) {
        cm.data[i].hits = {60};
        cm.data[i].delay += 120 * i;
      }
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, delay);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 2201004, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  leidianshu();
  return true;
}();