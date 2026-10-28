#pragma once

#include "src/client/game/game_mob.h"
#include <string>
class MobLogicSystem {
private:
  static bool run_animate(GameMob &g_mob);
  static void run_collision();
  static void run_mv();
  static void run_state_machine(GameMob &g_mob);
  static void run_alpha(GameMob &g_mob);
  static void run_debuff(GameMob &g_mob);

public:
  enum class ActionEnum {
    kStand,
    kJump,
    kMove,
    kSwim,
    kFly,
    kHit,
    kDie,
    kRevive,
  };
  static ActionEnum load_action_type(const std::u16string &action);

  static bool run();
  static std::optional<SDL_FRect> load_rect(const GameMob &g_mob);
  static std::optional<SDL_FPoint> load_head(const GameMob &g_mob);
};