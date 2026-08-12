#pragma once

#include "src/client/game/game_mob.h"
#include <string>
class mob_logic_system {
private:
  static bool run_animate(game_mob &g_mob);
  static void run_collision();
  static void run_mv();
  static void run_state_machine(game_mob &g_mob);
  static void run_alpha(game_mob &g_mob);

public:
  enum class action_enum {
    stand,
    jump,
    move,
    swim,
    fly,
    hit,
    die,
    revive,
  };
  static action_enum load_action_type(const std::u16string &action);
  static void run_revive(game_mob &g_mob);

  static bool run();
  static std::optional<SDL_FRect> load_rect(const game_mob &g_mob);
  static std::optional<SDL_FPoint> load_head(const game_mob &g_mob);
};