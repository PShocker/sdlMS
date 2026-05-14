#include "server_mob_instance.h"
#include "server_scene_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server/server_mob.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <optional>

void server_mob_instance::load_mob(server_scene &scene) {
  std::flat_map<uint32_t, server_mob> data;
  auto map_id = scene.map_id;
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
    server_mob s_mob;
    auto mob_node = val[0];
    s_mob.id =
        static_cast<wz::Property<std::u16string> *>(mob_node->get_child(u"id"))
            ->get();
    s_mob.fh =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"fh"))->get();
    s_mob.rx0 =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"rx0"))->get();
    s_mob.rx1 =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"rx1"))->get();

    auto x = static_cast<wz::Property<int> *>(mob_node->get_child(u"x"))->get();
    auto m_fh = fhs.at(s_mob.fh);
    auto y = m_fh.k.value() * x + m_fh.intercept.value();
    s_mob.pos = {static_cast<float>(x), static_cast<float>(y)};
    s_mob.page = m_fh.page;
    // default action
    mob_node = wz_resource::mob->find(s_mob.id + u".img");
    if (mob_node->get_child(u"info")->get_child("flySpeed")) {
      s_mob.action = u"fly";
      s_mob.type = server_mob::mob_type::fly;
    } else {
      s_mob.action = u"stand";
      s_mob.type = server_mob::mob_type::stand;
    }
    data[s_mob.index] = s_mob;
  }
  scene.mobs = data;
}