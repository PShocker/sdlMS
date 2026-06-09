#include "drop_logic_system.h"
#include "src/client/game/game_drop.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/window/window.h"
#include <cmath>
#include <ranges>

void drop_logic_system::pick(game_character &g_character) {}

void drop_logic_system::pick(game_pet &g_pet) {}

void drop_logic_system::run_state_machine(game_drop &drop) {
  switch (drop.type) {
  case game_drop::fly: {
    auto delta_time = window::delta_time / 1000.0;
    drop.vspeed = drop.vspeed + delta_time * 2000;
    auto dy = drop.vspeed * delta_time;
    drop.pos.y += dy;
    if (drop.pos.y >= drop.goal.y && drop.vspeed > 0) {
      drop.pos.x = drop.goal.x;
      drop.pos.y = drop.goal.y;
      drop.type = game_drop::land;
      drop.rotate = 0;
      return;
    }
    drop.rotate += (float)window::delta_time;
    break;
  }
  case game_drop::land: {
    double t = fmod((double)window::dt_now / 4100, 1.0);
    auto dy = (1.0 - fabs(2.0 * t - 1.0)) * 8.0 - 4.0;
    drop.pos.y = drop.goal.y + dy;
    break;
  }
  case game_drop::pick: {
    break;
  }
  }
}

bool drop_logic_system::run() {
  for (auto &drop : drop_game_instance::data | std::views::values) {
    run_state_machine(drop);
  }
  return true;
}