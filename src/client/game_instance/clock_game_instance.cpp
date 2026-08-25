#include "clock_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

void clock_game_instance::load(wz::Node *image) {
  data = {};
  if (auto n = image->get_child(u"clock")) {
    SDL_FRect r;
    r.x = static_cast<wz::Property<int> *>(n->get_child(u"x"))->get();
    r.y = static_cast<wz::Property<int> *>(n->get_child(u"y"))->get();
    r.w = static_cast<wz::Property<int> *>(n->get_child(u"width"))->get();
    r.h = static_cast<wz::Property<int> *>(n->get_child(u"height"))->get();
    data.push_back(r);
  }
}

void clock_game_instance::load(uint32_t map_id) {
  auto map_node = wz_resource::load_map_node(map_id);
  load(map_node);
}