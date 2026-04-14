#include "map_info_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <string>

void map_info_game_instance::load(uint32_t map_id) {
  map_info.clear();

  auto map_node = wz_resource::load_map_node(map_id);
  auto map_info_node = map_node->get_child(u"info");
  for (auto [key, val] : *map_info_node->get_children()) {
    auto node = val[0];
    switch (node->type) {
    case wz::Type::NotSet:
    case wz::Type::Null: {
      map_info.insert({key, nullptr});
      break;
    }
    case wz::Type::Int: {
      map_info.insert({key, static_cast<wz::Property<int> *>(node)->get()});
      break;
    }
    case wz::Type::String: {
      map_info.insert(
          {key, static_cast<wz::Property<std::u16string> *>(node)->get()});
      break;
    }
    default: {
      break;
    }
    }
  }
}