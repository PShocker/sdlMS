#pragma once

#include "src/client/game/game_obj.h"

namespace sdlms {

class ObjLogicSystem {
 public:
  static void RunAnimate(GameObj& g_obj);
  static void RunAlpha(GameObj& g_obj);
  static bool Run();
};

}  // namespace sdlms
