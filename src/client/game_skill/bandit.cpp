
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
  game_skill g_skill;
  g_skill.climb = true;
  g_skill.id = u"4201001";
  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"4201001"; });
    if (it != ski.end()) {
      ski.erase(it);
      character_logic_system::self_hspeed_max -= 100;
      character_logic_system::self_hspeed_min += 100;
    }
  };

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &ski = skill_game_instance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = window::dt_now + g_skill.duration;
    ski.push_back(g_skill);

    character_logic_system::self_hspeed_max += 100;
    character_logic_system::self_hspeed_min -= 100;

    if (client_id != 0) {
      return;
    }

    const SDL_FPoint lt = {-250, -150};
    const SDL_FPoint rb = {250, 150};

    skill_game_instance::skis()[u"4201001"].cd = window::dt_now + 1000;

    auto &sf = character_game_instance::self;
    auto g_r = skill_game_instance::load_ski_r(u"4201001");
    auto players = character_logic_system::run_buff_check(sf, g_r);
    auto ckt =
        skill_game_instance::create_skill_payload(players, 4201001, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void huixuanzhan() {
  game_skill g_skill;
  g_skill.id = u"4201003";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = character_game_instance::self;
    character_logic_system::run_action(sf, u"savage");
    auto g_r = skill_game_instance::load_ski_r(u"4201003");
    auto cm = character_logic_system::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = skill_game_instance::load_ski_time(sf);
      // Create and send attack payload
      auto atk_count =
          skill_game_instance::load_ski_atk_count(u"4201003", ski_lv);

      cm.data[0].hits = {60};

      cat = skill_game_instance::create_attack_payload(cm, sf.pos, delay);
      cat.payload[0]->effect = "420.img/skill/4201003/hit/0";
      for (int i = 0; i < atk_count; i++) {
        cat.payload[0]->attack->delay += 120;
        client_request::send_to_host(cat);
      }
    }
    auto ckt = skill_game_instance::create_skill_payload(4201003, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void shentoushu() {
  game_skill g_skill;
  g_skill.id = u"4001002";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = character_game_instance::self;
    character_logic_system::run_attack_action(sf, false);
    auto g_r = skill_game_instance::load_ski_r(u"4001002");
    auto cm = character_logic_system::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = skill_game_instance::load_ski_time(sf);
      // Create and send attack payload
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, delay);
      client_request::send_to_host(cat);
      // drop
      ClientCreateDropT ccd;
      ccd.map_id = scene_system_instance::map_id;
      auto &mob = cm.data[0].mob;
      auto dts = server_mob_instance::create_mob_dts(mob.id, mob.pos.x,
                                                     mob.pos.y, mob.page);
      for (auto dt : dts) {
        ccd.drops.push_back(std::make_unique<DropT>(dt));
      }
      if (!ccd.drops.empty()) {
        client_request::send_to_host(ccd);
      }
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 4001002, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  qinggong();
  huixuanzhan();
  return true;
}();