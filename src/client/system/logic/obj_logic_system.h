#pragma once

#include "src/client/game/game_obj.h"
class obj_logic_system {
public:
  static void run_animate(game_obj &g_obj);
  static void run_alpha(game_obj &g_obj);
  static bool run();
};