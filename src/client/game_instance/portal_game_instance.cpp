#include "portal_game_instance.h"
#include "src/client/game/game_portal.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <string>

const static std::u16string pt_list[] = {
    u"sp",  u"pi",  u"pv", u"pc",  u"pg",  u"tp",  u"ps",   u"pgi",
    u"psi", u"pcs", u"ph", u"psh", u"pcj", u"pci", u"pcig", u"pshg"};

void portal_game_instance::load(uint32_t map_id) {
  data = {};

  auto map_node = wz_resource::load_map_node(map_id);
  auto map_portal_node = map_node->get_child(u"portal");
  for (auto [key, val] : *map_portal_node->get_children()) {
    auto id = std::stoi(std::string{key.begin(), key.end()});
    auto portal_node = val[0];
    auto pt =
        static_cast<wz::Property<int> *>(portal_node->get_child(u"pt"))->get();
    if (!(pt < 0 || pt >= sizeof(pt_list))) {
      game_portal g_portal;
      g_portal.tm =
          static_cast<wz::Property<int> *>(portal_node->get_child(u"tm"))
              ->get();
      g_portal.pn = static_cast<wz::Property<std::u16string> *>(
                        portal_node->get_child(u"pn"))
                        ->get();
      g_portal.tn = static_cast<wz::Property<std::u16string> *>(
                        portal_node->get_child(u"tn"))
                        ->get();
      auto x =
          static_cast<wz::Property<int> *>(portal_node->get_child(u"x"))->get();
      auto y =
          static_cast<wz::Property<int> *>(portal_node->get_child(u"y"))->get();
      g_portal.pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};

      if (g_portal.pt == 7) {
        g_portal.pt = 2;
      }

      g_portal.path = u"MapHelper.img/portal/game/" + pt_list[pt];

      g_portal.path2 = u"/default";

      portal_game_instance::data.emplace_back(g_portal);
    }
  }
}