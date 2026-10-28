#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_npc.h"
#include <optional>
#include <string>
class NpcLogicSystem {
private:
  static bool run_animate(GameNpc &g_npc);
  static bool run_move(GameNpc &g_npc);
  static bool run_duration(GameNpc &g_npc);
  static bool run_chatballoon(GameNpc &g_npc);

  enum class ActionEnum {
    stand,
    move,
  };
  static ActionEnum load_action_type(GameNpc &g_npc);

public:
  static bool run();
};