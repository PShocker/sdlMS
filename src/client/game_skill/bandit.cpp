
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
#include "src/server/server_instance/server_character_instance.h"
#include <memory>

static void qinggong() {
  game_skill g_skill;
  g_skill.climb = true;
  g_skill.id = u"4101001";
  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"4101001"; });
    if (it != ski.end()) {
      ski.erase(it);
      character_logic_system::self_hspeed_max -= 100;
      character_logic_system::self_hspeed_min += 100;
    }
  };

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](int ski_lv) mutable {
    skill_game_instance::skis()[u"4101001"].cd = window::dt_now + 1000;

    auto &ski = skill_game_instance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = window::dt_now + g_skill.duration;
    ski.push_back(g_skill);

    character_logic_system::self_hspeed_max += 100;
    character_logic_system::self_hspeed_min -= 100;

    auto &sf = character_game_instance::self;
    ClientCharacterAttackT cat;
    auto ckt = skill_game_instance::create_skill_payload(cat, 4101001, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, sf);
    client_request::send_to_host(ckt);
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  qinggong();
  return true;
}();