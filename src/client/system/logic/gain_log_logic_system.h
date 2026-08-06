#pragma once

#include "src/client/game/game_gain_log.h"
#include "src/client/game/game_mob.h"
#include <string>
class gain_log_logic_system {
private:
  static void run_destroy();

public:
  static bool run();
};