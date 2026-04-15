#include "map_info_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <string>

void map_info_game_instance::load(uint32_t map_id) {
  auto map_node = wz_resource::load_map_node(map_id);
  map_info = map_node->get_child(u"info");
}