#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_NPC_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_NPC_LOGIC_SYSTEM_H_

#include "src/client/game/game_npc.h"

#include <SDL3/SDL_events.h>

#include <optional>
#include <string>
namespace sdlms {

class NpcLogicSystem {
 private:
  static bool RunAnimate(GameNpc& g_npc);
  static bool RunMove(GameNpc& g_npc);
  static bool RunDuration(GameNpc& g_npc);
  static bool RunChatballoon(GameNpc& g_npc);

  enum class ActionEnum {
    kStand,
    kMove,
  };
  static ActionEnum LoadActionType(GameNpc& g_npc);

 public:
  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_NPC_LOGIC_SYSTEM_H_
