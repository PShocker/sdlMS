#include "seat_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

void seat_game_instance::load(uint32_t map_id) {
  data = {};
  auto map_node = wz_resource::load_map_node(map_id);
  auto map_seat_node = map_node->get_child(u"seat");
  if (map_seat_node != nullptr) {
    for (auto [key, val] : *map_seat_node->get_children()) {
      auto vec = static_cast<wz::Property<wz::WzVec2D> *>(val[0])->get();
      data.push_back({static_cast<float>(vec.x), static_cast<float>(vec.y)});
    }
  }
}