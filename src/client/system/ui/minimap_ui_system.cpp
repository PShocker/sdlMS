#include "minimap_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/ui/ui_button.h"
#include "src/client/ui/ui_minimap.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>

void minimap_ui_system::load_minimap(uint32_t map_id) {
  ui_minimap minimap;
  auto string_node = wz_resource::string->find(u"Map.img");
  auto map_path = wz_resource::load_map_path(map_id);
  for (auto [key, val] : *string_node->get_children()) {
    if (val[0]->get_child(map_path)) {
      string_node = val[0]->get_child(map_path);
      auto name_node = string_node->get_child(u"mapName");
      minimap.map_name =
          static_cast<wz::Property<std::u16string> *>(name_node)->get();
      auto street_node = string_node->get_child(u"streetName");
      minimap.map_street =
          static_cast<wz::Property<std::u16string> *>(street_node)->get();
      break;
    }
  }
  minimap.map_id = map_id;
  minimap.pos = SDL_FPoint{0, 0};
  ui.insert({0, minimap});
}

void minimap_ui_system::load_button() {
  ui_button min_button;
  ui_button max_button;
  ui.insert({1, min_button});
  ui.insert({1, max_button});
}

minimap_ui_system::minimap_ui_system(uint32_t map_id) {
  ui.clear();
  load_minimap(map_id);
  load_button();
}