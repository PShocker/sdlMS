#include "src/client/game_instance/tooltip_game_instance.h"

#include "src/common/wz/wz_resource.h"
namespace sdlms {


void TooltipGameInstance::Load(uint32_t map_id) {
  data_ = {};
  auto map_node = WzResource::LoadMapNode(map_id);
  if (map_node->get_child(u"ToolTip")) {
  }
}

}  // namespace sdlms
