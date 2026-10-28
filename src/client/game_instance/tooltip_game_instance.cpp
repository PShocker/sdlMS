#include "tooltip_game_instance.h"
#include "src/common/wz/wz_resource.h"

void TooltipGameInstance::load(uint32_t map_id) {
  data = {};
  auto map_node = WzResource::load_map_node(map_id);
  if (map_node->get_child(u"ToolTip")) {
  }
}