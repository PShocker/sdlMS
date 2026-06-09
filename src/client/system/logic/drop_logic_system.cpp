#include "drop_logic_system.h"
#include "src/client/game/game_drop.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/window/window.h"
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
    if (drop.pos.y >= drop.goal.y) {
      drop.pos.x = drop.goal.x;
      drop.pos.y = drop.goal.y;
      drop.type = game_drop::land;
      return;
    }
    break;
  }
  case game_drop::land: {
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