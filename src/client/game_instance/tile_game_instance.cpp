#include "tile_game_instance.h"
#include "src/client/game/game_tile.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <string>

void tile_game_instance::load(uint32_t map_id) {
  data = {};

  auto map_node = wz_resource::load_map_node(map_id);
  uint8_t map_layer = 0;
  for (auto i : {u"0", u"1", u"2", u"3", u"4", u"5", u"6", u"7"}) {
    auto layer_node = map_node->get_child(i);
    auto tS = layer_node->get_child(u"info")->get_child(u"tS");
    if (tS == nullptr) {
      continue;
    }
    auto tS2 = static_cast<wz::Property<std::u16string> *>(tS)->get();
    for (auto [key, val] : *layer_node->get_child(u"tile")->get_children()) {
      game_tile g_tile;
      auto tile_node = val[0];

      auto u = static_cast<wz::Property<std::u16string> *>(
                   tile_node->get_child(u"u"))
                   ->get();
      auto no =
          static_cast<wz::Property<int> *>(tile_node->get_child(u"no"))->get();
      auto x =
          static_cast<wz::Property<int> *>(tile_node->get_child(u"x"))->get();
      auto y =
          static_cast<wz::Property<int> *>(tile_node->get_child(u"y"))->get();

      g_tile.pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};

      auto no2 = std::to_string(no);
      auto no3 = std::u16string{no2.begin(), no2.end()};

      g_tile.path = u"Tile/" + tS2 + u".img/" + u + u"/" + no3;

      auto zm =
          static_cast<wz::Property<int> *>(tile_node->get_child(u"zM"))->get();

      g_tile.z = zm;
      tile_game_instance::data[map_layer].insert({g_tile.z, g_tile});
    }
    map_layer++;
  }
}