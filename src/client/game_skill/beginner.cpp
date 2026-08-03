
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include <cstdint>
#include <memory>
#include <string>

static void ThreeSnail() {
  game_skill g_skill;
  g_skill.id = u"0001000";
  g_skill.use = [](int ski_lv) {
    game_triangle tri = {
        {SDL_FPoint{-350, -100}, SDL_FPoint{-350, 100}, SDL_FPoint{0, -28}}};
    auto &self = character_game_instance::self;
    character_logic_system::run_action(self, u"swingO1");
    auto cm = character_logic_system::run_attack_check(self, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = skill_game_instance::load_ski_time(self);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"000.img/skill/0001000/level/";
    path += {ski_lvl2.begin(), ski_lvl2.end()};
    path += u"/ball";
    auto pos = self.pos;
    pos.y -= 28;
    auto page = self.page;
    SDL_FPoint goal = pos;
    if (self.flip) {
      goal.x += 350;
    } else {
      goal.x -= 350;
    }
    auto cct = ball_game_instance::create_ball_payload(cm, pos, goal, delay,
                                                       page, 700, path);
    client_request::send_to_host(cct);
    server_ball_instance::handle_server_b(cct.payload);

    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = ball_game_instance::load_ball_time(cct);
      // Create and send attack payload
      cm.data[0].y = 0;
      cm.data[0].hits = {10};
      cat = skill_game_instance::create_attack_payload(cm, self.pos, d);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 1000, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void Recover() {
  game_skill g_skill;
  g_skill.climb = true;
  g_skill.id = u"0001001";

  static uint64_t now;
  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"0001001"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.frame = []() {
    if (now <= window::dt_now) {
      const auto &pos = character_game_instance::self.pos;
      now = window::dt_now + 1000;
      AttackT at;
      at.x = pos.x - 10;
      at.y = pos.y - 30;
      at.type = AttackEnum_Blue;
      at.num = 20;
      server_mob_instance::handle_s_attack(0, at);
      character_stat_game_instance::hp_point += at.num;
    }
  };

  g_skill.use = [g_skill](int ski_lv) mutable {
    skill_game_instance::skis()[u"0001001"].cd = window::dt_now + 10000;

    auto &ski = skill_game_instance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = window::dt_now + g_skill.duration;
    ski.push_back(g_skill);

    auto &self = character_game_instance::self;
    ClientCharacterAttackT cat;
    auto ckt = skill_game_instance::create_skill_payload(cat, 1001, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void NimbleFeet() {
  game_skill g_skill;
  g_skill.climb = true;
  g_skill.id = u"0001002";
  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"0001002"; });
    if (it != ski.end()) {
      ski.erase(it);
      character_logic_system::self_hspeed_max -= 100;
      character_logic_system::self_hspeed_min += 100;
    }
  };

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](int ski_lv) mutable {
    skill_game_instance::skis()[u"0001002"].cd = window::dt_now + 1000;

    auto &ski = skill_game_instance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = window::dt_now + g_skill.duration;
    ski.push_back(g_skill);

    character_logic_system::self_hspeed_max += 100;
    character_logic_system::self_hspeed_min -= 100;

    auto &self = character_game_instance::self;
    ClientCharacterAttackT cat;
    auto ckt = skill_game_instance::create_skill_payload(cat, 1002, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  ThreeSnail();
  Recover();
  NimbleFeet();
  return true;
}();