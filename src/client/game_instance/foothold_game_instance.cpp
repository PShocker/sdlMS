#include "foothold_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <vector>

std::vector<game_foothold>
foothold_game_instance::load_obj_fhs(uint32_t map_id) {
  std::vector<game_foothold> r;
  auto map_node = wz_resource::load_map_node(map_id);
  uint8_t map_layer = 0;
  for (auto i : {u"0", u"1", u"2", u"3", u"4", u"5", u"6", u"7"}) {
    auto layer_node = map_node->get_child(i);
    for (auto [key, val] : *layer_node->get_child(u"obj")->get_children()) {
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
      auto path = u"Obj/" + oS + u".img/" + l0 + u"/" + l1 + u"/" + l2;
      auto x =
          static_cast<wz::Property<int> *>(obj_node->get_child(u"x"))->get();
      auto y =
          static_cast<wz::Property<int> *>(obj_node->get_child(u"y"))->get();
      auto pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};

      obj_node = wz_resource::map->find(path);
    }
    map_layer++;
  }
  return r;
}

std::flat_map<int32_t, game_foothold>
foothold_game_instance::load(uint32_t map_id) {
  std::flat_map<int32_t, game_foothold> data;

  auto map_node = wz_resource::load_map_node(map_id);
  auto map_foothold_node = map_node->get_child(u"foothold");
  for (auto [page, val0] : *map_foothold_node->get_children()) {
    for (auto [zmass, val1] : *val0[0]->get_children()) {
      for (auto [key, val2] : *val1[0]->get_children()) {

        auto foothold_node = val2[0];
        game_foothold g_foothold;

        g_foothold.id = std::stoi(std::string{key.begin(), key.end()});
        g_foothold.page = std::stoi(std::string{page.begin(), page.end()});
        g_foothold.zmass = std::stoi(std::string{zmass.begin(), zmass.end()});

        g_foothold.next =
            static_cast<wz::Property<int> *>(foothold_node->get_child(u"next"))
                ->get();
        g_foothold.prev =
            static_cast<wz::Property<int> *>(foothold_node->get_child(u"prev"))
                ->get();
        g_foothold.x1 =
            static_cast<wz::Property<int> *>(foothold_node->get_child(u"x1"))
                ->get();
        g_foothold.x2 =
            static_cast<wz::Property<int> *>(foothold_node->get_child(u"x2"))
                ->get();
        g_foothold.y1 =
            static_cast<wz::Property<int> *>(foothold_node->get_child(u"y1"))
                ->get();
        g_foothold.y2 =
            static_cast<wz::Property<int> *>(foothold_node->get_child(u"y2"))
                ->get();

        g_foothold.l = std::min(g_foothold.x1, g_foothold.x2);
        g_foothold.r = std::max(g_foothold.x1, g_foothold.x2);
        g_foothold.t = std::min(g_foothold.y1, g_foothold.y2);
        g_foothold.b = std::max(g_foothold.y1, g_foothold.y2);

        if (g_foothold.x1 != g_foothold.x2) {
          // 斜线
          g_foothold.k = ((float)g_foothold.y2 - (float)g_foothold.y1) /
                         ((float)g_foothold.x2 - (float)g_foothold.x1);
          g_foothold.intercept =
              g_foothold.y1 - g_foothold.k.value() * g_foothold.x1;
        }
        data.emplace(g_foothold.id, g_foothold);
      }
    }
  }
  return data;
}