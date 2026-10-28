#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_OBJ_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_OBJ_LOGIC_SYSTEM_H_

#include "src/client/game/game_obj.h"
namespace sdlms {

class ObjLogicSystem {
 public:
  static void RunAnimate(GameObj& g_obj);
  static void RunAlpha(GameObj& g_obj);
  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_OBJ_LOGIC_SYSTEM_H_
