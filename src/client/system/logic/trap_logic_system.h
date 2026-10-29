#pragma once

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
