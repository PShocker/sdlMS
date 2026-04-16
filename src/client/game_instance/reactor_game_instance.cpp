#include "reactor_game_instance.h"
#include "src/client/game/game_reactor.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

void reactor_game_instance::load(uint32_t map_id) {
  auto map_node = wz_resource::load_map_node(map_id);
  auto map_reactor_node = map_node->get_child(u"reactor");
  if (map_reactor_node != nullptr) {
    for (auto [key, val] : *map_reactor_node->get_children()) {
      auto reactor_node = val[0];
      game_reactor g_reactor;
      g_reactor.id = static_cast<wz::Property<std::u16string> *>(
                         reactor_node->get_child(u"id"))
                         ->get();
    }
  }
}