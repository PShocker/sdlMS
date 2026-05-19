#pragma once

#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include <vector>
class effect_logic_system {
private:
  static bool run_afterimage(game_effect &g_effect);

  static void run_animate(std::vector<game_effect> &v);

public:
  static bool run();
};