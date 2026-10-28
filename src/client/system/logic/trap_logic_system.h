#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_TRAP_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_TRAP_LOGIC_SYSTEM_H_

#include "src/client/game/game_obj.h"

namespace sdlms {

class TrapLogicSystem {
 private:
  static void RunMove(GameObj& g_trap);
  static bool RunAttackCheck(GameObj& g_trap);
  static void RunCollision();

 public:
  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_TRAP_LOGIC_SYSTEM_H_
