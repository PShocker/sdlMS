#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_drop.h"
#include <cstdint>
#include <vector>
class drop_logic_system {
private:
  static void run_destroy();
  static void run_state_machine(game_drop &drop);

public:
  static bool run();
};