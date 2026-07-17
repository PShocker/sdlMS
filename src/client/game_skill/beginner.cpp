
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/common.h"
#include <memory>

static void three_snail() {
  game_skill g_skill;
  g_skill.id = u"0001000";
  g_skill.use = []() {
    game_triangle tri = {{SDL_FPoint{-350, -100}, {-350, 100}, {0, 0}}};
    auto &self = character_game_instance::self;
    auto mobs = character_logic_system::run_attack_check(self, tri);
    CharacterBallT cbt;
    cbt.ski_id = 1000;
    BallT bt;
    bt.x1 = self.pos.x;
    bt.y1 = self.pos.y;
    bt.page = self.page;
    if (!mobs.empty()) {
      bt.mob = true;
      bt.mob_index = mobs[0].mob.index;
      cbt.ball.push_back(std::make_unique<BallT>(bt));
    } else {
      bt.mob = false;
      cbt.ball.push_back(std::make_unique<BallT>(bt));
    }
    ball_game_instance::load(cbt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void heal() {
  game_skill g_skill;
  g_skill.id = u"0001001";
  g_skill.use = []() {};
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  three_snail();
  heal();
  return true;
}();