
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include <cstdint>
#include <memory>
#include <string>

static void ThreeSnail() {
  game_skill g_skill;
  g_skill.id = u"0001000";
  g_skill.use = []() {
    game_triangle tri = {
        {SDL_FPoint{-350, -100}, SDL_FPoint{-350, 100}, SDL_FPoint{0, -30}}};
    auto &self = character_game_instance::self;
    character_logic_system::run_action(self, u"swingO1");
    auto cm = character_logic_system::run_attack_check(self, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = skill_game_instance::load_ski_time(self);
    auto ski_lvl = job_skill_game_instance::load_skill_level(u"0001000");
    auto ski_lvl2 = std::to_string(ski_lvl);
    std::u16string path = u"000.img/skill/0001000/level/";
    path += {ski_lvl2.begin(), ski_lvl2.end()};
    path += u"/ball";
    auto pos = self.pos;
    pos.y -= 30;
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

    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = ball_game_instance::load_ball_time(cct);
      // Create and send attack payload
      cat = skill_game_instance::create_attack_payload(cm, self.pos, d);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 1000, ski_lvl);
    server_character_instance::handle_ski(ckt.ski_id, ski_lvl, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void Recover() {
  game_skill g_skill;
  g_skill.id = u"0001001";
  g_skill.lv = job_skill_game_instance::load_skill_level(u"0001001");

  static uint64_t now;
  static uint8_t count;
  g_skill.use = [g_skill]() {
    now = 0;
    count = 0;
    skill_game_instance::ski.push_back(g_skill);
  };

  g_skill.frame = []() {
    if (count >= 5) {
      return false;
    }
    if (now <= window::dt_now) {
      now = window::dt_now + 2000;
      AttackT at;
      at.type = AttackEnum_Blue;
      at.num = 20;
      server_mob_instance::handle_s_attack(0, at);
      character_stat_game_instance::hp_point += at.num;
      return true;
    }

    return true;
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void SpeedUp() {
  game_skill g_skill;
  g_skill.id = u"0001002";
  g_skill.lv = job_skill_game_instance::load_skill_level(u"0001002");
  static uint64_t now;
  g_skill.use = [g_skill]() {
    now = window::dt_now + 5000;
    character_logic_system::self_hspeed_max += 100;
    character_logic_system::self_hspeed_min -= 100;
    skill_game_instance::ski.push_back(g_skill);
  };
  g_skill.frame = []() {
    if (window::dt_now >= now) {
      character_logic_system::self_hspeed_max -= 100;
      character_logic_system::self_hspeed_min += 100;
      return false;
    }
    return true;
  };
}

[[maybe_unused]] static const bool r = [] {
  ThreeSnail();
  Recover();
  SpeedUp();
  return true;
}();