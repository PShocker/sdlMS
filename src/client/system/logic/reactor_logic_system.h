#pragma once

#include "src/client/game/game_reactor.h"
class reactor_logic_system {
private:
  static bool run_animate(game_reactor &g_reactor);
  static void run_alpha(game_reactor &g_reactor);

public:
  static bool run();
};