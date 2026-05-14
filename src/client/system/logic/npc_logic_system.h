#pragma once

#include "src/client/game/game_npc.h"
class npc_logic_system {
private:
  static bool run_animate(game_npc &g_npc);
  static bool run_move(game_npc &g_npc);
  static bool run_action(game_npc &g_npc);
  enum class action_enum {
    stand,
    move,
  };
  static action_enum load_action_type(game_npc &g_npc);

public:
  static bool run();
};