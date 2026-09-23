
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
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
static void guanchuanjian() {
  game_skill g_skill;
  g_skill.id = u"3201004";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = character_game_instance::self;
    character_logic_system::run_attack_action(sf, true);
    auto delay = skill_game_instance::load_ski_time(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"320.img/skill/3201004/ball";
    auto pos = sf.pos;
    pos.y -= 30;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    const auto w = 400;
    const auto h = 72;
    if (sf.flip) {
      goal.x += w;
    } else {
      goal.x -= w;
    }
    check_mobs cm;
    auto cct = ball_game_instance::create_ball_payload(cm, pos, goal, delay,
                                                       page, 800, path);
    client_request::send_to_host(cct);
    server_ball_instance::handle_server_b(cct.payload);

    ClientCharacterAttackT cat;
    SDL_FPoint lt{-w, -h};
    SDL_FPoint rb{0, 0};
    auto g_r = skill_game_instance::load_r(lt, rb, sf.pos, sf.flip);
    cm = character_logic_system::run_attack_check(sf, g_r);
    if (!cm.data.empty()) {
      for (auto &data : cm.data) {
        cct.payload->ball->mob_index = data.mob.index;
        auto mob_pos = mob_game_instance::data.at(data.mob.index).mob.pos;
        cct.payload->ball->y1 = mob_pos.y;
        cct.payload->ball->x2 = 0;
        cct.payload->ball->y2 = 0;
        data.delay = ball_game_instance::load_ball_time(cct);
        data.hits = {100};
      }
      // Create and send attack payload
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, 0);
      for (auto &data : cat.payload) {
        data->effect = "Afterimage/hit.img/sword2";
      }
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(4111004, ski_lv);
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
    auto &ski = skill_game_instance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = window::dt_now + g_skill.duration;
    ski.push_back(g_skill);

    if (client_id != 0) {
      return;
    }
    skill_game_instance::skis()[u"3101003"].cd = window::dt_now + 1000;

    auto ckt = skill_game_instance::create_skill_payload(3101003, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  guanchuanjian();
  wuyingjian();
  return true;
}();