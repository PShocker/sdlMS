#include "item_logic_system.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"

bool ItemLogicSystem::run() {
  auto v = ItemGameInstance::buff;
  for (auto i : v) {
    if (i.destroy <= Window::dt_now) {
      ItemGameInstance::unuse_buff_item(i.id);
    }
  }
  return true;
}