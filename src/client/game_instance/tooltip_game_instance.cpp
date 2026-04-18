#include "tooltip_game_instance.h"
#include "src/common/wz/wz_resource.h"

void tooltip_game_instance::load(uint32_t map_id) {
  data = {};
  auto map_node = wz_resource::load_map_node(map_id);
  if (map_node->get_child(u"ToolTip")) {
  }
}