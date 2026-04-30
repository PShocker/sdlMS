#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_input.h"
#include <cstdint>
class character_logic_system {
private:
public:
  static bool run();
  static void input_action(game_input &g_input);
  static void input_skill(game_input &g_input);
};