#include "reactor_logic_system.h"
#include "src/client/game_instance/reactor_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <ranges>
#include <string>

bool reactor_logic_system::run_animate(game_reactor &g_reactor) {
  bool finish = false;
  if (g_reactor.hit) {
    auto node = wz_resource::reactor->find(g_reactor.id + u".img");
    node = node->get_child(std::to_string(g_reactor.state));
    node = node->get_child(u"hit");
    auto count = node->children_count();
    node = node->get_child(std::to_string(g_reactor.ani_index));

    if (node->type == wz::Type::UOL) {
      node = static_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }

    int32_t delay = 100;
    auto delay_node = node->get_child(u"delay");
    if (delay_node != nullptr) {
      delay = static_cast<wz::Property<int> *>(delay_node)->get();
    }
    g_reactor.ani_time += window::delta_time;
    if (g_reactor.ani_time > delay) {
      g_reactor.ani_time = 0;
      g_reactor.ani_index += 1;
      finish = g_reactor.ani_index >= count;
      g_reactor.ani_index = g_reactor.ani_index % count;
    }
  }
  return finish;
}

bool reactor_logic_system::run() {
  for (auto &rs : reactor_game_instance::data) {
    for (auto &r : rs) {
      if (run_animate(r)) {
        r.state += 1;
        r.ani_index = 0;
        r.ani_time = 0;
        r.hit = false;
      }
    }
  }
  return true;
}