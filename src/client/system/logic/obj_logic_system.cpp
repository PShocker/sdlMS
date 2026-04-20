#include "obj_logic_system.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <ranges>
#include <string>

bool obj_logic_system::run() {
  for (auto &o : obj_game_instance::data) {
    for (auto &g_obj : o | std::views::values) {
      auto obj_node = wz_resource::map->find(g_obj.path);
      if (obj_node->children_count() > 1) {
        g_obj.ani_time += window::delta_time;

        auto index = std::to_string(g_obj.ani_index);
        obj_node = obj_node->get_child(index);

        int32_t delay = 100;
        auto delay_node = obj_node->get_child(u"delay");
        if (delay_node != nullptr) {
          delay = static_cast<wz::Property<int> *>(delay_node)->get();
        }

        if (g_obj.ani_time >= delay) {
          g_obj.ani_time = 0;
          g_obj.ani_index += 1;
          g_obj.ani_index = g_obj.ani_index % obj_node->children_count();
        }
      }
    }
  }
  return true;
}