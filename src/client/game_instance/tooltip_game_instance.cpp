#include "tooltip_game_instance.h"
#include "src/common/wz/wz_resource.h"

void TooltipGameInstance::Load(uint32_t map_id) {
  data = {};
  auto map_node = WzResource::LoadMapNode(map_id);
  if (map_node->get_child(u"ToolTip")) {
  }
}