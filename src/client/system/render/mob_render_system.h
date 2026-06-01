#pragma once

#include "src/client/game/game_mob.h"
class mob_render_system {
private:
  static bool render_mob(game_mob &g_mob);
  static bool render_gauge(game_mob &g_mob);
  static bool render_effect_back(game_mob &g_mob);

public:
  static bool render(game_mob &g_mob);
};