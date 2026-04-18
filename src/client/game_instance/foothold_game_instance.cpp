#include "foothold_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

void foothold_game_instance::load(uint32_t map_id) {
  data.clear();
  
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
          g_foothold.wall = false;
        } else {
          g_foothold.wall = true;
        }
        data.insert({g_foothold.id, g_foothold});
      }
    }
  }
}