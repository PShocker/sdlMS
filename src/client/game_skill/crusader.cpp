
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
#include <vector>

static void douqijizhong() {
  game_skill g_skill;
  g_skill.climb = true;
  g_skill.id = u"1111000";
  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"1111000"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.frame = []() { return; };

  g_skill.state = [](game_character *c, int index) {
    game_effect e = {
        .id = u"1111000",
        .index = static_cast<uint32_t>(index),
        .time = 0,
        .delay = window::dt_now,
        .type = game_effect::effect_type::skill_custom,
        .pos = std::nullopt,
        .z = false,
        .data = c,
    };
    auto &eff = c->effect;
    std::erase_if(eff, [&](const auto &ef) { return ef.id == e.id; });
    eff.push_back(e);
  };

  g_skill.effect = [](SDL_FPoint p, game_effect *e, bool f) { return false; };

  g_skill.action = [](game_skill *s, std::u16string ski_id) {
    if (ski_id.empty()) {
      return;
    }
    s->start += 1;
    auto &ccs = character_logic_system::ccs;
    StateT st;
    st.state = StateEnum_BUFF_SKILL;
    st.val = 1111000;
    st.sub_val = s->start;
    ccs.payload.push_back(std::make_unique<StateT>(st));
    return;
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &ski = skill_game_instance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 0;
    g_skill.destroy = 0;
    g_skill.start = 0;
    ski.push_back(g_skill);

    skill_game_instance::skis()[u"1111000"].cd = window::dt_now + 1000;

    auto &sf = character_game_instance::self;
    auto ckt = skill_game_instance::create_skill_payload(1111000, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);

    auto &ccs = character_logic_system::ccs;
    StateT st;
    st.state = StateEnum_BUFF_SKILL;
    st.val = 1111000;
    st.sub_val = 0;
    ccs.payload.push_back(std::make_unique<StateT>(st));
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  douqijizhong();
  return true;
}();