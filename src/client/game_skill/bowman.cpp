
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
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

static void duanhunjian() {
  game_skill g_skill;
  g_skill.id = u"3001001";
  g_skill.use = [](int ski_lv) {
    game_triangle tri = {
        {
            SDL_FPoint{-350, -100},
            SDL_FPoint{-350, 100},
            SDL_FPoint{0, -28},
        },
    };
    auto &sf = character_game_instance::self;
    character_logic_system::run_action(sf, u"swingO1");
    auto cm = character_logic_system::run_attack_check(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = skill_game_instance::load_ski_time(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"300.img/skill/3001001/ball";
    auto pos = sf.pos;
    pos.y -= 28;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
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
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, d);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 3001001, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, sf);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void shuangfeizhan() {
  game_skill g_skill;
  g_skill.id = u"4001003";
  g_skill.fall = true;
  g_skill.use = [](int ski_lv) {
    const game_triangle tri = {
        {
            SDL_FPoint{-350, -100},
            SDL_FPoint{-350, 100},
            SDL_FPoint{0, -28},
        },
    };
    auto &sf = character_game_instance::self;
    auto weapon_type = equip_game_instance::load_weapon_type(sf);
    if (!(weapon_type == equip_game_instance::weapon_type::BOW ||
          weapon_type == equip_game_instance::weapon_type::CROSSBOW)) {
      return;
    }
    auto ball = package_game_instance::load_active_ball();
    if (ball == nullptr) {
      return;
    }
    auto ball_id = (*ball)->id;
    character_logic_system::run_action(sf, u"swingO1");
    auto cm = character_logic_system::run_attack_check(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = skill_game_instance::load_ski_time(sf);
    auto pre = ball_id.substr(0, 4);
    std::u16string path = u"Consume/" + pre + u".img/" + ball_id + u"/bullet";
    auto pos = sf.pos;
    pos.y -= 28;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 350;
    } else {
      goal.x -= 350;
    }
    auto cct = ball_game_instance::create_ball_payload(cm, pos, goal, delay,
                                                       page, 700, path);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = ball_game_instance::load_ball_time(cct);
      // Create and send attack payload
      cm.data[0].hits = {15, 15};
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, d, 120);
      client_request::send_to_host(cat);
    }
    for (auto i : {0, 1}) {
      cct.payload->ball->y1 += i * 5;
      cct.payload->ball->y2 += i * 5;
      client_request::send_to_host(cct);
      server_ball_instance::handle_server_b(cct.payload);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 4001003, ski_lv);
    if (!ckt.payload.empty()) {
      ckt.payload[1]->y += 10;
    }
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, sf);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  duanhunjian();

  return true;
}();