#include "item_logic_system.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"

bool item_logic_system::run() {
  auto v = item_game_instance::buff;
  for (auto i : v) {
    if (i.destroy <= window::dt_now) {
      item_game_instance::unuse_buff_item(i.id);
    }
  }
  return true;
}