#pragma once

#include "src/client/game/game_obj.h"
class obj_logic_system {
private:
  static void run_animate(game_obj &g_obj);
  static void run_alpha(game_obj &g_obj);

public:
  static bool run();
};