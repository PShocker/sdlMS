
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include <memory>
#include <string>

static void ThreeSnail() {
  game_skill g_skill;
  g_skill.id = u"0001000";
  g_skill.use = []() {
    game_triangle tri = {{SDL_FPoint{-350, -100}, {-350, 100}, {0, 0}}};
    auto &self = character_game_instance::self;
    character_logic_system::run_action(self, u"swingO1");
    auto cm = character_logic_system::run_attack_check(self, tri);
    auto delay = skill_game_instance::load_ski_time(self);
    auto ski_lvl = job_skill_game_instance::load_skill_level(u"0001000");
    auto ski_lvl2 = std::to_string(ski_lvl);
    std::u16string path = u"000.img/skill/0001000/level/";
    path += {ski_lvl2.begin(), ski_lvl2.end()};
    path += u"/ball";
    auto pos = self.pos;
    pos.y -= 30;
    auto cct = ball_game_instance::create_ball_payload(cm, pos, delay, path);
    client_request::send_to_host(cct);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void Recover() {
  game_skill g_skill;
  g_skill.id = u"0001001";
  g_skill.use = []() {};
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  ThreeSnail();
  Recover();
  return true;
}();