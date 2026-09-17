
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <vector>

static void jianyu() {
  game_skill g_skill;
  g_skill.id = u"3111002";

  struct data_pack {
    game_character *c;
    struct data_special {
      float x1;
      float y1;
      float x2;
      float y2;
    };
    std::vector<data_special> specs;
  };

  g_skill.effect = [](SDL_FPoint p, game_effect *e, bool f) {
    data_pack *dp;
    const std::type_info &tp = e->data.type();
    if (tp == typeid(game_character *)) {
      auto g_character = std::any_cast<game_character *>(e->data);
      data_pack dp;
      dp.c = g_character;
      e->data = dp;
    }
    dp = std::any_cast<data_pack>(&e->data);

    return true;
  };

  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = character_game_instance::self;
    character_logic_system::run_attack_action(sf, false);
    auto g_r = skill_game_instance::load_ski_r(u"3111002");
    auto cm = character_logic_system::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = skill_game_instance::load_ski_time(sf);
      // Create and send attack payload
      for (int i = 0; i < cm.data.size(); i++) {
        cm.data[i].hits = {60};
        cm.data[i].delay += 120 * i;
      }
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, delay);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 3111002, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  jianyu();
  return true;
}();