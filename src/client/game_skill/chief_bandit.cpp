
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_drop.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/effect_logic_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <vector>

static void jinqianzhadan() {
  game_skill g_skill;
  g_skill.id = u"4211004";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = character_game_instance::self;
    character_logic_system::run_action(sf, u"prone2");
    auto g_r = skill_game_instance::load_ski_r(u"4211004");
    auto cm = character_logic_system::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    ClientDropFadeT ccdf;
    uint64_t meso = 0;
    for (const auto &[k, v] : drop_game_instance::data) {
      if (v.type != game_drop::drop_enum::land) {
        continue;
      }
      if (SDL_PointInRectFloat(&v.pos, &g_r)) {
        if (v.data->id == u"00000000") {
          meso += item_game_instance::load_item_num(v.data);
          ccdf.random_id.push_back(v.random_id);
        }
      }
    }
    if (!ccdf.random_id.empty()) {
      client_request::send_to_host(ccdf);
    }
    if (!cm.data.empty() && meso > 0) {
      auto delay = skill_game_instance::load_ski_time(sf);
      // Create and send attack payload
      for (auto &data : cm.data) {
        data.hits = {(int64_t)meso};
      }
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, delay);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 4211004, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void luoyezhan() {
  game_skill g_skill;
  g_skill.id = u"4211001";

  struct data_pack {
    game_character *c;
    std::vector<game_effect> es;
  };

  g_skill.effect = [](SDL_FPoint p, game_effect *e, bool f) {
    data_pack *dp;
    const std::type_info &tp = e->data.type();
    if (tp == typeid(game_character *)) {
      data_pack d;
      auto g_character = std::any_cast<game_character *>(e->data);
      d.c = g_character;
      game_effect ef = {
          .id = u"4211001",
          .index = 0,
          .time = 0,
          .delay = 0,
          .type = game_effect::effect_type::skill_use,
          .pos = std::nullopt,
          .data = g_character,
      };
      d.es.emplace_back(ef);
      ef.order = 0;
      d.es.emplace_back(ef);
      e->data = d;
    }
    dp = std::any_cast<data_pack>(&e->data);

    if (effect_logic_system::run_effect(*e)) {
      return false;
    }
    auto g_character = std::any_cast<game_character *>(e->data);
    effect_render_system::render_effect(g_character->pos, *e);
    return false;
  };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = character_game_instance::self;
    character_logic_system::run_attack_action(sf, false);
    auto g_r = skill_game_instance::load_ski_r(u"4211001");
    auto cm = character_logic_system::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = skill_game_instance::load_ski_time(sf);
      // Create and send attack payload
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, delay);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 4211001, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  jinqianzhadan();
  luoyezhan();
  return true;
}();