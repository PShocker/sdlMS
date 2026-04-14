#include "ladderrope_game_instance.h"
#include "src/common/wz/wz_resource.h"

void ladderrope_game_instance::load(uint32_t map_id) {
  auto map_node = wz_resource::load_map_node(map_id);
  auto map_ladder_node = map_node->get_child(u"ladderRope");
  for (auto [key, val] : *map_ladder_node->get_children()) {
  }
}