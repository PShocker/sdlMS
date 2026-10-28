
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
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include <memory>

static void qinggong() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"4201001";
  g_skill.end = []() {
    auto &ski = SkillGameInstance::ski;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill &s) { return s.id == u"4201001"; });
    if (it != ski.end()) {
      ski.erase(it);
      CharacterLogicSystem::self_hspeed_max -= 100;
      CharacterLogicSystem::self_hspeed_min += 100;
    }
  };

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &ski = SkillGameInstance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = Window::dt_now + g_skill.duration;
    ski.push_back(g_skill);

    CharacterLogicSystem::self_hspeed_max += 100;
    CharacterLogicSystem::self_hspeed_min -= 100;

    if (client_id != 0) {
      return;
    }

    const SDL_FPoint lt = {-250, -150};
    const SDL_FPoint rb = {250, 150};

    SkillGameInstance::skis()[u"4201001"].cd = Window::dt_now + 1000;

    auto &sf = CharacterGameInstance::self;
    auto g_r = SkillGameInstance::load_ski_r(u"4201001");
    auto players = CharacterLogicSystem::run_buff_check(sf, g_r);
    auto ckt =
        SkillGameInstance::create_skill_payload(players, 4201001, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };

  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

static void huixuanzhan() {
  GameSkill g_skill;
  g_skill.id = u"4201003";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_action(sf, u"savage");
    auto g_r = SkillGameInstance::load_ski_r(u"4201003");
    auto cm = CharacterLogicSystem::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::load_ski_time(sf);
      // Create and send attack payload
      auto atk_count =
          SkillGameInstance::load_ski_atk_count(u"4201003", ski_lv);

      cm.data[0].hits = {60, 1, 1, 1, 1, 1, 1};
      std::string effect = "420.img/skill/4201003/hit/0";
      std::string sound = "Skill.img/4201003/Hit";
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, delay, 120,
                                                       effect, sound);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::create_skill_payload(4201003, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

static void shentoushu() {
  GameSkill g_skill;
  g_skill.id = u"4001002";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_attack_action(sf, false);
    auto g_r = SkillGameInstance::load_ski_r(u"4001002");
    auto cm = CharacterLogicSystem::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::load_ski_time(sf);
      // Create and send attack payload
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, delay);
      ClientRequest::send_to_host(cat);
      // drop
      ClientCreateDropT ccd;
      ccd.map_id = SceneSystemInstance::map_id;
      auto &mob = cm.data[0].mob;
      auto dts = ServerMobInstance::create_mob_dts(mob.id, mob.pos.x,
                                                     mob.pos.y, mob.page);
      for (auto dt : dts) {
        ccd.drops.push_back(std::make_unique<DropT>(dt));
      }
      if (!ccd.drops.empty()) {
        ClientRequest::send_to_host(ccd);
      }
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 4001002, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  qinggong();
  huixuanzhan();
  return true;
}();