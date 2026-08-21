#include "item_buff_logic_system.h"
#include "src/client/game_instance/item_buff_game_instance.h"
#include "src/client/window/window.h"

bool item_buff_logic_system::run() {
  auto v = item_buff_game_instance::data;
  for (auto i : v) {
    if (i.destroy <= window::dt_now) {
      item_buff_game_instance::end(i.id);
    }
  }
  return true;
}