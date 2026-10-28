#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_BACKGRND_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_BACKGRND_LOGIC_SYSTEM_H_

#include "src/client/game/game_backgrnd.h"
namespace sdlms {

class BackgrndLogicSystem {
 private:
  static void RunOffset(GameBackgrnd& g_backgrnd);
  static void RunAnimate(GameBackgrnd& g_backgrnd);

 public:
  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_BACKGRND_LOGIC_SYSTEM_H_
