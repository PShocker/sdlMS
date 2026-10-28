#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_npc.h"
#include <optional>
#include <string>
class NpcLogicSystem {
private:
  static bool RunAnimate(GameNpc &g_npc);
  static bool RunMove(GameNpc &g_npc);
  static bool RunDuration(GameNpc &g_npc);
  static bool RunChatballoon(GameNpc &g_npc);

  enum class ActionEnum {
    kStand,
    kMove,
  };
  static ActionEnum LoadActionType(GameNpc &g_npc);

public:
  static bool Run();
};