#pragma once

#include "src/client/game/game_effect.h"
#include <vector>
class EffectLogicSystem {
private:
  static bool run_damage(GameEffect &g_effect);
  static bool run_skill_affected(GameEffect &g_effect);
  static bool run_skill_hit(GameEffect &g_effect);
  static bool run_skill_region(GameEffect &g_effect);

  static void run_animate(std::vector<GameEffect> &v);

public:
  static bool run_effect(GameEffect &g_effect);
  static bool run_skill_use(GameEffect &g_effect);

  static bool run();
};