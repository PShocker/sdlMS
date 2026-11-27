#include "src/client/game_instance/foothold_game_instance.h"

#include "src/common/wz/wz_resource.h"

#include <wz/Property.h>

#include <vector>

namespace sdlms {

std::flat_map<int32_t, GameFoothold> FootholdGameInstance::Load(
    wz::Node* node) {
  std::flat_map<int32_t, GameFoothold> data;
  auto map_node = node;
  auto map_foothold_node = map_node->get_child(u"foothold");
  for (auto [page, val0] : *map_foothold_node->get_children()) {
    for (auto [zmass, val1] : *val0[0]->get_children()) {
      for (auto [key, val2] : *val1[0]->get_children()) {
        auto foothold_node = val2[0];
        GameFoothold g_foothold;

        g_foothold.id = std::stoi(std::string{key.begin(), key.end()});
        g_foothold.page = std::stoi(std::string{page.begin(), page.end()});
        g_foothold.zmass = std::stoi(std::string{zmass.begin(), zmass.end()});

        g_foothold.next =
            static_cast<wz::Property<int>*>(foothold_node->get_child(u"next"))
                ->get();
        g_foothold.prev =
            static_cast<wz::Property<int>*>(foothold_node->get_child(u"prev"))
                ->get();
        g_foothold.x1 =
            static_cast<wz::Property<int>*>(foothold_node->get_child(u"x1"))
                ->get();
        g_foothold.x2 =
            static_cast<wz::Property<int>*>(foothold_node->get_child(u"x2"))
                ->get();
        g_foothold.y1 =
            static_cast<wz::Property<int>*>(foothold_node->get_child(u"y1"))
                ->get();
        g_foothold.y2 =
            static_cast<wz::Property<int>*>(foothold_node->get_child(u"y2"))
                ->get();

        g_foothold.l = std::min(g_foothold.x1, g_foothold.x2);
        g_foothold.r = std::max(g_foothold.x1, g_foothold.x2);
        g_foothold.t = std::min(g_foothold.y1, g_foothold.y2);
        g_foothold.b = std::max(g_foothold.y1, g_foothold.y2);

        // 095 CAttrFoothold：drag/force 在 WZ 中是百分数，除以 100 后使用。
        auto load_int = [foothold_node](const char16_t* name, int fallback) {
          auto* child = foothold_node->get_child(name);
          if (child == nullptr || child->type != wz::Type::Int) {
            return fallback;
          }
          return static_cast<wz::Property<int>*>(child)->get();
        };
        g_foothold.forbid_fall_down = load_int(u"forbidFallDown", 0) != 0;
        g_foothold.cant_through = load_int(u"cantThrough", 0) != 0;
        g_foothold.walk = static_cast<float>(load_int(u"walk", 100)) / 100.0f;
        g_foothold.drag = static_cast<float>(load_int(u"drag", 100)) / 100.0f;
        g_foothold.force = static_cast<float>(load_int(u"force", 0)) / 100.0f;

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

std::flat_map<int32_t, GameFoothold> FootholdGameInstance::Load(
    uint32_t map_id) {
  auto map_node = WzResource::LoadMapNode(map_id);
  return Load(map_node);
}

}  // namespace sdlms
