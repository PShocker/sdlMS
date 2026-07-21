
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
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <memory>

static void PowerStrke() {
  game_skill g_skill;
  g_skill.id = u"1001001";
  g_skill.use = [](int ski_lv) {
    auto &self = character_game_instance::self;
    character_logic_system::run_attack_action(self);
    SDL_FRect g_r = afterimage_game_instance::load_rect(self).value();
    auto cm = character_logic_system::run_attack_check(self, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = skill_game_instance::load_ski_time(self);
      // Create and send attack payload
      cat = skill_game_instance::create_attack_payload(cm, self.pos, delay);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 1001001, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void SlashBlast() {
  game_skill g_skill;
  g_skill.id = u"1001002";
  g_skill.use = [](int ski_lv) {
    auto &self = character_game_instance::self;
    character_logic_system::run_attack_action(self);
    SDL_FPoint lt = {-150, -100};
    SDL_FPoint rb = {0, 0};
    auto g_r = skill_game_instance::load_r(lt, rb, self.pos, self.flip);
    auto cm = character_logic_system::run_attack_check(self, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = skill_game_instance::load_ski_time(self);
      // Create and send attack payload
      cat = skill_game_instance::create_attack_payload(cm, self.pos, delay);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 1001002, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload,
                                          self);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  PowerStrke();
  SlashBlast();
  return true;
}();