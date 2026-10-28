#pragma once
#include "src/client/game/game_pet.h"

class PetLogicSystem {
private:
  static void run_state_machine();
  static void run_animate(GamePet &pet);

public:
  static bool run();
};