#include "input_system.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include <flat_map>
#include <string>

void input_system::handle_ui_input(game_input &g_input) {
  static const std::flat_map<std::string, void (*)()> fns = {
      {"worldMap", worldmap_ui_system::toggle},
  };
  fns.at(g_input.val)();
}

void input_system::handle_item_input(game_input &g_input) {
  static const std::flat_map<std::string, void (*)()> fns = {
      {"worldMap", worldmap_ui_system::toggle},
  };
  fns.at(g_input.val)();
}