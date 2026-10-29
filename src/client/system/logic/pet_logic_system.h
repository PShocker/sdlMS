#pragma once

#include "src/client/game/game_pet.h"

namespace sdlms {

class PetLogicSystem {
 private:
  static void RunStateMachine();
  static void RunAnimate(GamePet& pet);

 public:
  static bool Run();
};

}  // namespace sdlms
