#include "obj_logic_system.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <cstdint>
#include <ranges>
#include <string>

void obj_logic_system::run_animate(game_obj &g_obj) {
  if (g_obj.ani_count > 1) {
    g_obj.ani_time += window::delta_time;

    auto index = std::to_string(g_obj.ani_index);

    auto obj_node = wz_resource::map->find(g_obj.path);
    obj_node = obj_node->get_child(index);
    if (obj_node->type == wz::Type::UOL) {
      obj_node = static_cast<wz::Property<wz::WzUOL> *>(obj_node)->get_uol();
    }
    int32_t delay = 100;
    auto delay_node = obj_node->get_child(u"delay");
    if (delay_node != nullptr) {
      delay = static_cast<wz::Property<int> *>(delay_node)->get();
    }

    if (g_obj.ani_time >= delay) {
      g_obj.ani_time = 0;
      g_obj.ani_index += 1;
      g_obj.ani_index = g_obj.ani_index % g_obj.ani_count;
    }
  }
}

void obj_logic_system::run_alpha(game_obj &g_obj) {
  auto obj_node = wz_resource::map->find(g_obj.path);
  auto index = std::to_string(g_obj.ani_index);
  obj_node = obj_node->get_child(index);
  if (obj_node->type == wz::Type::UOL) {
    obj_node = static_cast<wz::Property<wz::WzUOL> *>(obj_node)->get_uol();
  }
  if (obj_node->get_child(u"a0")) {
    auto a0 =
        static_cast<wz::Property<int> *>(obj_node->get_child(u"a0"))->get();
    auto a1 =
        static_cast<wz::Property<int> *>(obj_node->get_child(u"a1"))->get();
    auto delay_node = obj_node->get_child(u"delay");
    auto delay = static_cast<wz::Property<int> *>(delay_node)->get();

    float t = (float)g_obj.ani_time / (float)delay;
    g_obj.alpha = a0 + (a1 - a0) * t;
  }
}

bool obj_logic_system::run() {
  for (auto &o : obj_game_instance::data) {
    for (auto &g_obj : o | std::views::values) {
      run_animate(g_obj);
      run_alpha(g_obj);
    }
  }
  return true;
}