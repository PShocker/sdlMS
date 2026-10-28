#pragma once

#include "src/client/game/game_reactor.h"
class ReactorLogicSystem {
private:
  static bool run_animate(GameReactor &g_reactor);
  static void run_alpha(GameReactor &g_reactor);

public:
  static bool run();
};