#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_drop.h"
#include <cstdint>
#include <vector>
class DropLogicSystem {
private:
  static void run_destroy();
  static void run_state_machine(GameDrop &drop);

public:
  static bool run();
};