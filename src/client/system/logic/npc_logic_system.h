#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_npc.h"
#include <optional>
#include <string>
class npc_logic_system {
private:
  static bool run_animate(game_npc &g_npc);
  static bool run_move(game_npc &g_npc);
  static bool run_duration(game_npc &g_npc);
  static bool run_chatballoon(game_npc &g_npc);

  static std::optional<std::u16string> cursor_in();

  enum class action_enum {
    stand,
    move,
  };
  static action_enum load_action_type(game_npc &g_npc);

public:
  static bool run();
};