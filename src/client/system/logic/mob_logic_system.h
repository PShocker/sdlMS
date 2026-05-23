#pragma once

#include "src/client/game/game_mob.h"
class mob_logic_system {
private:
  static bool run_animate(game_mob &g_mob);
  static void run_collision();
  static void run_logic();
  static void run_state_machine(game_mob &g_mob);

public:
  static bool run();
  static std::optional<SDL_FRect> load_rect(const game_mob &g_mob);
};