#include "portal_logic_system.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/window/window.h"

bool portal_logic_system::run() {
  for (auto &portal : portal_game_instance::data) {
    portal.ani_time += window::delta_time;
    if (portal.ani_time >= 100) {
      portal.ani_index += 1;
      portal.ani_time = 0;
    }
  }
  return true;
}