#include "npc_game_instance.h"
#include "foothold_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <array>
#include <flat_set>
#include <string>

wz::Node *npc_game_instance::load_link_npc_node(const std::u16string &id) {
  auto npc_node = wz_resource::npc->find(id + u".img");
  while (npc_node->find(u"info/link")) {
    auto link_node = npc_node->find(u"info/link");
    auto link = static_cast<wz::Property<std::u16string> *>(link_node)->get();
    npc_node = wz_resource::npc->find(link + u".img");
  }
  return npc_node;
}

void npc_game_instance::load(uint32_t map_id) {
  data = {};

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
    g_npc.id =
        static_cast<wz::Property<std::u16string> *>(npc_node->get_child(u"id"))
            ->get();
    g_npc.fh =
        static_cast<wz::Property<int> *>(npc_node->get_child(u"fh"))->get();
    g_npc.rx0 =
        static_cast<wz::Property<int> *>(npc_node->get_child(u"rx0"))->get();
    g_npc.rx1 =
        static_cast<wz::Property<int> *>(npc_node->get_child(u"rx1"))->get();
    auto x = static_cast<wz::Property<int> *>(npc_node->get_child(u"x"))->get();
    auto y = static_cast<wz::Property<int> *>(npc_node->get_child(u"y"))->get();
    g_npc.pos = {static_cast<float>(x), static_cast<float>(y)};

    // default action
    g_npc.action = u"stand";

    auto layer = foothold_game_instance::data.at(g_npc.fh).page;
    data[layer].push_back(g_npc);
  }
}