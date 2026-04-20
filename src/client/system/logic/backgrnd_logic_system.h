#pragma once

#include "src/client/game/game_backgrnd.h"
class backgrnd_logic_system {
private:
  static void run_offset(game_backgrnd &g_backgrnd);
  static void run_animate(game_backgrnd &g_backgrnd);

public:
  static bool run();
};