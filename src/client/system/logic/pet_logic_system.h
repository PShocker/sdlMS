#pragma once
#include "src/client/game/game_pet.h"

class pet_logic_system {
private:
  static void run_state_machine();
  static void run_animate(game_pet &pet);

public:
  static bool run();
};