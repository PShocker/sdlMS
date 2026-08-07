#include "server_reactor_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <vector>

void server_reactor_instance::load_reactor(server_scene &scene) {
  std::vector<server_reactor> data;
  auto map_id = scene.map_id;
  auto map_node = wz_resource::load_map_node(map_id);

  auto map_reactor_node = map_node->get_child(u"reactor");
  if (map_reactor_node != nullptr) {
    int i = 0;
    for (auto [key, val] : *map_reactor_node->get_children()) {
      auto reactor_node = val[0];
      server_reactor reactor;
      reactor.id = static_cast<wz::Property<std::u16string> *>(
                       reactor_node->get_child(u"id"))
                       ->get();
      auto x = static_cast<wz::Property<int> *>(reactor_node->get_child(u"x"))
                   ->get();
      auto y = static_cast<wz::Property<int> *>(reactor_node->get_child(u"y"))
                   ->get();
      reactor.pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};
      reactor.index = i;
      reactor.act = 0;
      data.emplace_back(reactor);
      i++;
    }
  }
  scene.reactors = data;
}