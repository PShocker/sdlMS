#pragma once

#include "src/client/game/game_obj.h"
class TrapLogicSystem {
private:
  static void run_move(GameObj &g_trap);
  static bool run_attack_check(GameObj &g_trap);
  static void run_collision();

public:
  static bool run();
};