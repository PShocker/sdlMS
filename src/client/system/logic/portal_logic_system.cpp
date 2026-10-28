#include "portal_logic_system.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/window/window.h"
#include <ranges>

bool PortalLogicSystem::run() {
  for (auto &portal : PortalGameInstance::data | std::views::values) {
    portal.ani_time += Window::delta_time;
    if (portal.ani_time >= 100) {
      portal.ani_index += 1;
      portal.ani_time = 0;
    }
  }
  return true;
}