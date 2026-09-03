
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/effect_logic_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include <algorithm>
#include <cstdint>
#include <string>

static void shunjianyidong() {
  game_skill g_skill;
  g_skill.id = u"2201001";

  g_skill.effect = [](SDL_FPoint p, game_effect *e, bool f) {
    e->id = u"BasicEff.img/Teleport";
    e->lvl = 1;
    e->pos = SDL_FPoint{0, 0};
    if (effect_logic_system::run_effect(*e)) {
      return false;
    }
    auto g_character = std::any_cast<game_character *>(e->data);
    effect_render_system::render_effect(g_character->pos, *e);
    e->id = u"2201001";
    return true;
  };

  g_skill.use = [g_skill](int ski_lv) {
    skill_game_instance::skis()[u"2201001"].cd = window::dt_now + 500;
    auto &sf = character_game_instance::self;

    auto up = keyboard_game_instance::find_key_by_val("up");
    auto down = keyboard_game_instance::find_key_by_val("down");
    auto left = keyboard_game_instance::find_key_by_val("left");
    auto right = keyboard_game_instance::find_key_by_val("right");

    auto x = sf.pos.x;
    auto y = sf.pos.y;

    if (window::keyboard_state[right]) {
      x += 200;
    } else if (window::keyboard_state[left]) {
      x -= 200;
    } else if (window::keyboard_state[down]) {
      y += 200;
    } else if (window::keyboard_state[up]) {
      y -= 200;
    }

    const auto &fhs = foothold_game_instance::data;
    auto border =
        map_info_game_instance::load_mr_border(scene_system_instance::map_id);
    auto [l, t, r, b] = border;
    x = std::clamp(x, l, r);
    y = std::clamp(y, t, b);
    if (y != sf.pos.y) {
      // 垂直移动
      auto ins = physic::fall_intersect_pos(sf.pos, {x, y}, fhs);
      if (!ins.empty()) {
        if (sf.pos.y < y) {
          sf.pos = ins.rbegin()->second.pos;
          character_logic_system::self_fh = ins.rbegin()->second.fh.id;
        } else {
          sf.pos = ins.begin()->second.pos;
          character_logic_system::self_fh = ins.begin()->second.fh.id;
        }
      }
    } else if (x != sf.pos.x) {
      auto ins = physic::fall_intersect_pos({x, y}, {x, y + 70}, fhs);
      if (!ins.empty()) {
        auto top = ins.begin()->first;
        sf.pos = ins.begin()->second.pos;
        character_logic_system::self_fh = ins.begin()->second.fh.id;
      } else {
        auto ins = physic::fall_intersect_pos({x, y}, {x, y - 70}, fhs);
        if (!ins.empty()) {
          auto top = ins.rbegin()->first;
          sf.pos = ins.rbegin()->second.pos;
          character_logic_system::self_fh = ins.begin()->second.fh.id;
        }
      }
    }

    character_logic_system::self_hspeed = 0;
    character_logic_system::self_vspeed = 0;

    ClientCharacterAttackT cat;
    auto ckt = skill_game_instance::create_skill_payload(cat, 2201001, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, sf);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  shunjianyidong();
  return true;
}();