#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_REACTOR_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_REACTOR_LOGIC_SYSTEM_H_

#include "src/client/game/game_reactor.h"
namespace sdlms {

class ReactorLogicSystem {
 private:
  static bool RunAnimate(GameReactor& g_reactor);
  static void RunAlpha(GameReactor& g_reactor);

 public:
  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_REACTOR_LOGIC_SYSTEM_H_
