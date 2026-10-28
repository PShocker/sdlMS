#pragma once

#include "src/client/game/game_backgrnd.h"
class BackgrndLogicSystem {
private:
  static void RunOffset(GameBackgrnd &g_backgrnd);
  static void RunAnimate(GameBackgrnd &g_backgrnd);

public:
  static bool Run();
};