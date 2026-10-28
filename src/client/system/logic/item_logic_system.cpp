#include "item_logic_system.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"
namespace sdlms {


bool ItemLogicSystem::Run() {
  auto v = ItemGameInstance::buff_;
  for (auto i : v) {
    if (i.destroy <= Window::dt_now_) {
      ItemGameInstance::UnuseBuffItem(i.id);
    }
  }
  return true;
}

}  // namespace sdlms
