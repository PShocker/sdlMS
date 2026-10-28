#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_DROP_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_DROP_LOGIC_SYSTEM_H_

#include "src/client/game/game_character.h"
#include "src/client/game/game_drop.h"

#include <cstdint>
#include <vector>
namespace sdlms {

class DropLogicSystem {
 private:
  static void RunDestroy();
  static void RunStateMachine(GameDrop& drop);

 public:
  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_DROP_LOGIC_SYSTEM_H_
