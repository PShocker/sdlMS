#pragma once

#include "src/client/game/game_reactor.h"
class ReactorLogicSystem {
private:
  static bool RunAnimate(GameReactor &g_reactor);
  static void RunAlpha(GameReactor &g_reactor);

public:
  static bool Run();
};