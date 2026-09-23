
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <vector>

// 310
static void baozhajian() {
  game_skill g_skill;
  g_skill.id = u"3101005";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    game_triangle tri = {
        {
            SDL_FPoint{-400, -100},
            SDL_FPoint{-400, 100},
            SDL_FPoint{0, -30},
        },
    };
    auto &sf = character_game_instance::self;
    character_logic_system::run_attack_action(sf, true);
    auto cm = character_logic_system::run_attack_check(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      cm.data[0].hits = {60};
    }
    auto delay = skill_game_instance::load_ski_time(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"310.img/skill/3101004/ball";
    auto pos = sf.pos;
    pos.y -= 30;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 400;
    } else {
      goal.x -= 400;
    }
    auto cct = ball_game_instance::create_ball_payload(cm, pos, goal, delay,
                                                       page, 700, path);
    client_request::send_to_host(cct);
    server_ball_instance::handle_server_b(cct.payload);

    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = ball_game_instance::load_ball_time(cct);
      auto mob_pos = cm.data[0].mob.pos;
      auto g_r = skill_game_instance::load_ski_r(u"3101005", mob_pos, sf.flip);
      cm = character_logic_system::run_attack_check(mob_pos, g_r, 2);
      // Create and send attack payload
      for (int i = 0; i < cm.data.size(); i++) {
        auto &data = cm.data[i];
        if (i == 0) {
          data.hits = {100};
        } else {
          data.hits = {80};
          data.delay = 100;
        }
      }
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, d);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 3101005, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void wuyingjian() {
  game_skill g_skill;
  g_skill.climb = true;
  g_skill.id = u"3101003";
  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"3101003"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.action = [](game_skill *s, std::u16string ski_id) {
    auto b = package_game_instance::load_active_ball();
    if (b == nullptr) {
      return;
    }
    if (ski_id.empty()) {
      auto &sf = character_game_instance::self;
      if (sf.action == u"shoot1") {
        item_game_instance::add_item_num(*b, 1);
      }
    } else {
      auto ski_lv = job_skill_game_instance::load_ski_level(ski_id);
      auto num = skill_game_instance::load_ski_ball_num(ski_id, ski_lv);
      item_game_instance::add_item_num(*b, num);
    }
    return;
  };

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &sf = character_game_instance::self;
    character_logic_system::run_action(sf, u"alert2");

    auto &ski = skill_game_instance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = window::dt_now + g_skill.duration;
    ski.push_back(g_skill);

    skill_game_instance::skis()[u"3101003"].cd = window::dt_now + 1000;

    auto ckt = skill_game_instance::create_skill_payload(3101003, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  baozhajian();
  wuyingjian();
  return true;
}();