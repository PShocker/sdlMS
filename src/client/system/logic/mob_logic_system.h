#pragma once

#include "src/client/game/game_mob.h"
class mob_logic_system {
private:
  static bool run_animate(game_mob &g_mob);
  static void run_logic();
  static void run_state_machine(game_mob &g_mob);

public:
  static bool run();
};