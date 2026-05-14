#include "mob_game_instance.h"
#include "foothold_game_instance.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <vector>

wz::Node *mob_game_instance::load_link_mob_node(const std::u16string &id) {
  auto mob_node = wz_resource::mob->find(id + u".img");
  while (mob_node->find(u"info/link")) {
    auto link_node = mob_node->find(u"info/link");
    auto link = static_cast<wz::Property<std::u16string> *>(link_node)->get();
    mob_node = wz_resource::mob->find(link + u".img");
  }
  return mob_node;
}

void mob_game_instance::load(uint32_t map_id) {
  data = {};

  auto map_node = wz_resource::load_map_node(map_id);
  auto map_life_node = map_node->get_child(u"life");
  auto fhs = foothold_game_instance::load(map_id);

  for (auto [key, val] : *map_life_node->get_children()) {
    auto type =
        static_cast<wz::Property<std::u16string> *>(val[0]->get_child(u"type"))
            ->get();
    if (type == u"n") {
      continue;
    }
    game_mob g_mob;
    auto mob_node = val[0];
    g_mob.index = std::stoi(std::string{key.begin(), key.end()});
    g_mob.id =
        static_cast<wz::Property<std::u16string> *>(mob_node->get_child(u"id"))
            ->get();
    data[g_mob.index] = g_mob;
  }
}