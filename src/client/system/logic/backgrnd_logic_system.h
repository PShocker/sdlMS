#pragma once

#include "src/client/game/game_backgrnd.h"
class BackgrndLogicSystem {
private:
  static void run_offset(GameBackgrnd &g_backgrnd);
  static void run_animate(GameBackgrnd &g_backgrnd);

public:
  static bool run();
};