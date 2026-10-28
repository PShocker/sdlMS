#pragma once
#include "src/client/game/game_pet.h"

class PetLogicSystem {
private:
  static void RunStateMachine();
  static void RunAnimate(GamePet &pet);

public:
  static bool Run();
};