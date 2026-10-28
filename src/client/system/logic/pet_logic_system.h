#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_PET_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_PET_LOGIC_SYSTEM_H_

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

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_PET_LOGIC_SYSTEM_H_
