
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
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

static void yinshenshu() {
  game_skill g_skill;
  g_skill.climb = true;
  g_skill.id = u"4001001";
  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"4001001"; });
    if (it != ski.end()) {
      ski.erase(it);
      auto &ccs = character_logic_system::ccs;
      StateT st;
      st.state = StateEnum_BUFF_SKILL;
      st.val = 4001001;
      st.sub_val = 0;
      ccs.payload.push_back(std::make_unique<StateT>(st));
    }
  };

  g_skill.state = [](game_character *c, int index) {
    if (index == 1) {
      c->color = {255, 255, 255, 128};
    } else {
      c->color = {255, 255, 255, 255};
    }
  };
  g_skill.frame = [g_skill]() {
    auto &self = character_game_instance::self;
    self.color = {255, 255, 255, 128};
    auto type = character_logic_system::load_action_type(self);
    switch (type) {
    case character_logic_system::action_enum::attack:
    case character_logic_system::action_enum::skill: {
      g_skill.end();
      break;
    }
    default: {
      character_logic_system::self_invincible_cooldown = window::dt_now + 100;
      break;
    }
    }
  };

  g_skill.use = [g_skill](int ski_lv) mutable {
    skill_game_instance::skis()[u"4001001"].cd = window::dt_now + 1000;

    auto &ski = skill_game_instance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destory = window::dt_now + g_skill.duration;
    ski.push_back(g_skill);

    auto &self = character_game_instance::self;
    self.color = {255, 255, 255, 128};

    ClientCharacterAttackT cat;
    auto ckt = skill_game_instance::create_skill_payload(cat, 4001001, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);

    auto &ccs = character_logic_system::ccs;
    StateT st;
    st.state = StateEnum_BUFF_SKILL;
    st.val = 4001001;
    st.sub_val = 1;
    ccs.payload.push_back(std::make_unique<StateT>(st));
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void huibishu() {
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

static void shuangfeizhan() {
  game_skill g_skill;
  g_skill.id = u"4001003";
  g_skill.use = [](int ski_lv) {
    game_triangle tri = {
        {SDL_FPoint{-350, -100}, SDL_FPoint{-350, 100}, SDL_FPoint{0, -28}}};
    auto &self = character_game_instance::self;
    auto weapon_type = equip_game_instance::load_weapon_type(self);
    if (weapon_type != equip_game_instance::weapon_type::CLAW) {
      return;
    }
    auto ball_type = ball_game_instance::load_ball_type(self);
    auto ball_id = ball_game_instance::load_pkg_ball(2, ball_type);
    if (ball_id.empty()) {
      return;
    }
    character_logic_system::run_action(self, u"swingO1");
    auto cm = character_logic_system::run_attack_check(self, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = skill_game_instance::load_ski_time(self);
    std::u16string path = u"Consume/0207.img/" + ball_id + u"/bullet/";
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
    // 注意,需要再次发送一遍ball
    cct.payload->ball->delay += 60;
    client_request::send_to_host(cct);

    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = ball_game_instance::load_ball_time(cct);
      // Create and send attack payload
      cm.data[0].y = 0;
      cm.data[0].hits = {30, 60};
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

[[maybe_unused]] static const bool r = [] {
  yinshenshu();
  huibishu();
  return true;
}();