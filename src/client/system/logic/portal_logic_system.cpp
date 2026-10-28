#include "src/client/system/logic/portal_logic_system.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/window/window.h"
#include <ranges>
namespace sdlms {


bool PortalLogicSystem::Run() {
  for (auto &portal : PortalGameInstance::data_ | std::views::values) {
    portal.ani_time += Window::delta_time_;
    if (portal.ani_time >= 100) {
      portal.ani_index += 1;
      portal.ani_time = 0;
    }
  }
  return true;
}

}  // namespace sdlms
