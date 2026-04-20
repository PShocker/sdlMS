#include "obj_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_obj.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <string>

void obj_game_instance::load(uint32_t map_id) {
  data = {};
  auto map_node = wz_resource::load_map_node(map_id);
  uint8_t map_layer = 0;
  for (auto i : {u"0", u"1", u"2", u"3", u"4", u"5", u"6", u"7"}) {
    auto layer_node = map_node->get_child(i);
    for (auto [key, val] : *layer_node->get_child(u"obj")->get_children()) {
      game_obj g_obj;
      auto obj_node = val[0];
      auto oS = static_cast<wz::Property<std::u16string> *>(
                    obj_node->get_child(u"oS"))
                    ->get();
      auto l0 = static_cast<wz::Property<std::u16string> *>(
                    obj_node->get_child(u"l0"))
                    ->get();
      auto l1 = static_cast<wz::Property<std::u16string> *>(
                    obj_node->get_child(u"l1"))
                    ->get();
      auto l2 = static_cast<wz::Property<std::u16string> *>(
                    obj_node->get_child(u"l2"))
                    ->get();
      g_obj.path = u"Obj/" + oS + u".img/" + l0 + u"/" + l1 + u"/" + l2;

      auto x =
          static_cast<wz::Property<int> *>(obj_node->get_child(u"x"))->get();
      auto y =
          static_cast<wz::Property<int> *>(obj_node->get_child(u"y"))->get();
      auto z =
          static_cast<wz::Property<int> *>(obj_node->get_child(u"z"))->get();

      g_obj.pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};

      auto flip =
          static_cast<wz::Property<int> *>(obj_node->get_child(u"f"))->get();

      g_obj.flip = flip;

      auto load_z = std::stoi(std::string{key.begin(), key.end()});

      g_obj.z = z * 100000 + load_z * 10;

      obj_game_instance::data[map_layer].emplace(g_obj.z, g_obj);
    }
    map_layer++;
  }
}