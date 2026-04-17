#include "reactor_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "foothold_game_instance.h"
#include "src/client/game/game_reactor.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>

uint8_t reactor_game_instance::load_reactor_layer(SDL_FPoint pos) {
  for (auto [key, val] : foothold_game_instance::data) {
    if (val.l <= pos.x && val.r >= pos.x && pos.y >= val.t && pos.y <= val.b) {
      return val.page;
    }
  }
  return 0;
}

void reactor_game_instance::load(uint32_t map_id) {
  auto map_node = wz_resource::load_map_node(map_id);
  auto map_reactor_node = map_node->get_child(u"reactor");
  if (map_reactor_node != nullptr) {
    for (auto [key, val] : *map_reactor_node->get_children()) {
      auto reactor_node = val[0];
      game_reactor g_reactor;
      g_reactor.id = static_cast<wz::Property<std::u16string> *>(
                         reactor_node->get_child(u"id"))
                         ->get();
      auto x = static_cast<wz::Property<int> *>(reactor_node->get_child(u"x"))
                   ->get();
      auto y = static_cast<wz::Property<int> *>(reactor_node->get_child(u"y"))
                   ->get();
      g_reactor.pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};
      g_reactor.reactor_index = 0;
      g_reactor.ani_index = 0;
      g_reactor.ani_time = 0;
      g_reactor.reactor_time = static_cast<wz::Property<int> *>(
                                   reactor_node->get_child(u"reactorTime"))
                                   ->get();
      g_reactor.f =
          static_cast<wz::Property<int> *>(reactor_node->get_child(u"f"))
              ->get();
      auto layer = load_reactor_layer(g_reactor.pos);

      reactor_game_instance::data[layer].emplace_back(g_reactor);
    }
  }
}