
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <memory>

static void PowerStrke() {
  GameSkill g_skill;
  g_skill.id = u"1001001";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_attack_action(sf, false);
    SDL_FRect g_r = AfterimageGameInstance::load_rect(sf).value();
    auto cm = CharacterLogicSystem::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::load_ski_time(sf);
      // Create and send attack payload
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, delay);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 1001001, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

static void SlashBlast() {
  GameSkill g_skill;
  g_skill.id = u"1001002";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_attack_action(sf, false);
    auto g_r = SkillGameInstance::load_ski_r(u"1001002");
    auto cm = CharacterLogicSystem::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::load_ski_time(sf);
      // Create and send attack payload
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, delay);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 1001002, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  PowerStrke();
  SlashBlast();
  return true;
}();