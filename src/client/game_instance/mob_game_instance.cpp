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

std::array<std::vector<game_mob>, 8> mob_game_instance::load(uint32_t map_id) {
  std::array<std::vector<game_mob>, 8> data = {};

  auto map_node = wz_resource::load_map_node(map_id);
  auto map_life_node = map_node->get_child(u"life");
  for (auto [key, val] : *map_life_node->get_children()) {
    auto type =
        static_cast<wz::Property<std::u16string> *>(val[0]->get_child(u"type"))
            ->get();
    if (type == u"n") {
      continue;
    }

    game_mob g_mob;

    auto mob_node = val[0];

    g_mob.id =
        static_cast<wz::Property<std::u16string> *>(mob_node->get_child(u"id"))
            ->get();
    g_mob.fh =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"fh"))->get();
    g_mob.rx0 =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"rx0"))->get();
    g_mob.rx1 =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"rx1"))->get();

    auto x = static_cast<wz::Property<int> *>(mob_node->get_child(u"x"))->get();
    auto y = static_cast<wz::Property<int> *>(mob_node->get_child(u"y"))->get();
    g_mob.pos = {static_cast<float>(x), static_cast<float>(y)};

    // default action
    mob_node = wz_resource::mob->find(g_mob.id + u".img");
    if (mob_node->get_child(u"info")->get_child("flySpeed")) {
      g_mob.action = u"fly";
      g_mob.type = mob_type::Fly;
    } else {
      g_mob.action = u"stand";
      g_mob.type = mob_type::Stand;
    }

    auto layer = foothold_game_instance::data.at(g_mob.fh).page;
    data[layer].push_back(g_mob);
  }
  return data;
}

std::vector<uint32_t> mob_game_instance::load_visible_mob() {
  std::vector<uint32_t> r;
  auto &camera = camera_game_instance::camera;
  for (auto &mobs : data) {
    for (auto &mob : mobs) {
      auto &pos = mob.pos;
      if (SDL_PointInRectFloat(&pos, &camera)) {
        r.push_back(mob.index);
      }
    }
  }
  return r;
}