#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_pet.h"

namespace sdlms {

class PetLogicSystem {
 private:
  static void RunAnimate(GamePet& pet);
  static void RunFollow(GamePet& pet, const GameCharacter& owner, bool is_self);
  static void RunPick(GamePet& pet);
  static void RunStateMachine(GamePet& pet, const GameCharacter& owner);

 public:
  static bool Run();
};

}  // namespace sdlms
