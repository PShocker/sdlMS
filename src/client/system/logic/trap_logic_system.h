#pragma once

#include "src/client/game/game_obj.h"
class trap_logic_system {
private:
  static void run_move(game_obj &g_trap);
  static bool run_attack_check(game_obj &g_trap);
  static void run_collision();

public:
  static bool run();
};