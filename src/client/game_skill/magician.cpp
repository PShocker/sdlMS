
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"
#include "wz/Property.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

static void mfdan() {
  game_skill g_skill;
  g_skill.id = u"2001002";
  g_skill.use = [](int ski_lv) {
    game_triangle tri = {
        {SDL_FPoint{-400, -100}, SDL_FPoint{-400, 100}, SDL_FPoint{0, -30}}};
    auto &self = character_game_instance::self;
    const std::array actions = {u"swingO1", u"swingO2", u"swingO3"};
    std::uniform_int_distribution<> dis(0, actions.size() - 1);
    auto &gen = random_game_instance::gen;
    character_logic_system::run_action(self, actions[dis(gen)]);
    auto cm = character_logic_system::run_attack_check(self, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      cm.data[0].hits = {60};
    }
    auto delay = skill_game_instance::load_ski_time(self);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"200.img/skill/2001002/ball";
    auto pos = self.pos;
    pos.y -= 30;
    auto page = self.page;
    SDL_FPoint goal = pos;
    if (self.flip) {
      goal.x += 400;
    } else {
      goal.x -= 400;
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
    auto ckt = skill_game_instance::create_skill_payload(cat, 2001002, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void mfdun() {
  game_skill g_skill;
  g_skill.id = u"2001000";
  g_skill.hit = [](game_mob mob, uint64_t n) {
    auto ski_lv = job_skill_game_instance::load_ski_level(u"2001000");
    auto x_node = wz_resource::skill->find(u"200.img/skill/2001000/level");
    auto x = static_cast<wz::Property<int> *>(
                 x_node->get_child(std::to_string(ski_lv))->get_child(u"x"))
                 ->get();
  };
  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"2001000"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.use = [g_skill](int ski_lv) mutable {
    auto &self = character_game_instance::self;
    character_logic_system::run_action(self, u"alert2");

    auto &ski = skill_game_instance::ski;

    g_skill.end();
    g_skill.lv = ski_lv;
    ski.push_back(g_skill);

    ClientCharacterAttackT cat;
    auto ckt = skill_game_instance::create_skill_payload(cat, 2001000, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void mfsj() {
  game_skill g_skill;
  g_skill.id = u"2001003";
  g_skill.use = [](int ski_lv) {
    auto &self = character_game_instance::self;
    character_logic_system::run_attack_action(self);
    SDL_FPoint lt = {-300, -90};
    SDL_FPoint rb = {-25, 10};
    auto g_r = skill_game_instance::load_r(lt, rb, self.pos, self.flip);
    auto cm = character_logic_system::run_attack_check(self, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      auto delay = skill_game_instance::load_ski_time(self);
      // Create and send attack payload
      cm.data[0].hits = {60, 70};
      cat = skill_game_instance::create_attack_payload(cm, self.pos, delay);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 2001003, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void mfkaijia() {
  game_skill g_skill;
  g_skill.id = u"2001001";

  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"2001001"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.effect = [](SDL_FPoint p, game_effect e, bool f) {
    const int duration = 300;
    auto start = e.delay;
    auto dt = window::dt_now - start;
    if (dt <= duration) {
      float scale = 1 + 1.2 * (float)dt / duration;
      auto g_character = std::any_cast<game_character *>(e.data);
      auto character = *g_character;
      character.scale = scale;
      character.color.a = 192 * (1 - (float)dt / duration);
      character_render_system::render_character(character);
      return true;
    }
    return false;
  };

  g_skill.use = [g_skill](int ski_lv) mutable {
    auto &self = character_game_instance::self;
    auto &ski = skill_game_instance::ski;

    g_skill.end();
    g_skill.lv = ski_lv;
    ski.push_back(g_skill);

    character_logic_system::run_action(self, u"alert2");
    ClientCharacterAttackT cat;
    auto ckt = skill_game_instance::create_skill_payload(cat, 2001001, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void molitisheng() {
  game_skill g_skill;
  g_skill.id = u"2001099";

  g_skill.end = []() {
    auto &skis = skill_game_instance::skis();
    auto ski_lv = skis[u"2001099"].lv;
    character_stat_game_instance::ski_mp -= ski_lv;
  };
  g_skill.passive = [](int ski_lv) {
    auto &skis = skill_game_instance::skis();
    skis[u"2001099"].lv = ski_lv;
    character_stat_game_instance::ski_mp += ski_lv;
  };
  auto &skis = skill_game_instance::skis();

  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  mfdan();
  mfsj();
  mfkaijia();
  molitisheng();
  return true;
}();