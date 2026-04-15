#include "backgrnd_logic_system.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <ranges>
#include <string>

bool backgrnd_logic_system::run() {
  for (auto m : {backgrnd_game_instance::front, backgrnd_game_instance::back}) {
    for (auto &backgrnd : m | std::views::values) {
      if (backgrnd.ani == 1) {
        auto texture_node = wz_resource::map->find(backgrnd.path);
        auto ani_index = backgrnd.ani_index % texture_node->children_count();
        texture_node = texture_node->get_child(std::to_string(ani_index));

        auto delay =
            static_cast<wz::Property<int> *>(texture_node->get_child(u"delay"))
                ->get();

        backgrnd.ani_time += window::delta_time;
        if (backgrnd.ani_time >= delay) {
          backgrnd.ani_time = 0;
          backgrnd.ani_index = 0;
        }
      }
    }
  }
  return true;
}