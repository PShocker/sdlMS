
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
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <memory>
#include <vector>

static void erduantiao() {
  game_skill g_skill;
  g_skill.fall = true;
  g_skill.id = u"4111005";
  g_skill.end = []() {};

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &ski = skill_game_instance::ski;
    auto &sf = character_game_instance::self;
    auto action_type = character_logic_system::load_action_type(sf);
    if (action_type != character_logic_system::action_enum::jump) {
      return;
    }
    character_logic_system::self_vspeed -= 420;
    if (sf.flip == 1) {
      // 朝右
      character_logic_system::self_hspeed += 480;
    } else {
      character_logic_system::self_hspeed -= 480;
    }
    character_logic_system::self_two_jump_cooldown = UINT64_MAX;
    auto ckt = skill_game_instance::create_skill_payload(4111005, ski_lv);
    ckt.x = sf.pos.x;
    ckt.y = sf.pos.y;
    ckt.flip = sf.flip;
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0,
                                          ckt.x, ckt.y, ckt.flip);
    client_request::send_to_host(ckt);
    skill_game_instance::skis()[u"4111005"].cd = window::dt_now + 100;
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void dafeibiao() {
  game_skill g_skill;
  g_skill.id = u"4111004";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = character_game_instance::self;
    character_logic_system::run_action(sf, u"avenger");
    auto delay = skill_game_instance::load_ski_time(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"411.img/skill/4111004/ball";
    auto pos = sf.pos;
    pos.y -= 30;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 400;
    } else {
      goal.x -= 400;
    }
    check_mobs cm;
    auto cct = ball_game_instance::create_ball_payload(cm, pos, goal, delay,
                                                       page, 700, path);
    client_request::send_to_host(cct);
    server_ball_instance::handle_server_b(cct.payload);

    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = ball_game_instance::load_ball_time(cct);
      // Create and send attack payload
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, d);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 4111004, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  erduantiao();
  dafeibiao();
  return true;
}();