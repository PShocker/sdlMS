#include "npc_game_instance.h"
#include "foothold_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

void npc_game_instance::load(uint32_t map_id) {
  auto map_node = wz_resource::load_map_node(map_id);
  auto map_life_node = map_node->get_child(u"life");
  for (auto [key, val] : *map_life_node->get_children()) {
    auto type =
        static_cast<wz::Property<std::u16string> *>(val[0]->get_child(u"type"))
            ->get();
    if (type == u"m") {
      continue;
    }
    game_npc g_npc;

    auto npc_node = val[0];
    auto fh =
        static_cast<wz::Property<int> *>(npc_node->get_child(u"fh"))->get();
    auto rx0 =
        static_cast<wz::Property<int> *>(npc_node->get_child(u"rx0"))->get();
    auto rx1 =
        static_cast<wz::Property<int> *>(npc_node->get_child(u"rx1"))->get();

    g_npc.fh = fh;
    g_npc.rx0 = rx0;
    g_npc.rx1 = rx1;

    auto layer = foothold_game_instance::data.at(fh).page;
    data[layer].push_back(g_npc);
  }
}