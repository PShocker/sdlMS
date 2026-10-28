#include "src/client/game_instance/reactor_game_instance.h"

#include "src/client/game/game_reactor.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Property.h>

#include <cstdint>
namespace sdlms {


uint8_t ReactorGameInstance::LoadReactorLayer(SDL_FPoint pos) {
  for (const auto& [key, val] : FootholdGameInstance::data_) {
    if (val.l <= pos.x && val.r >= pos.x && pos.y >= val.t && pos.y <= val.b) {
      return val.page;
    }
  }
  return 0;
}

void ReactorGameInstance::Load(uint32_t map_id) {
  data_ = {};

  auto map_node = WzResource::LoadMapNode(map_id);
  auto map_reactor_node = map_node->get_child(u"reactor");
  if (map_reactor_node != nullptr) {
    uint8_t index = 0;
    for (auto [key, val] : *map_reactor_node->get_children()) {
      auto reactor_node = val[0];
      GameReactor g_reactor;
      g_reactor.id = static_cast<wz::Property<std::u16string>*>(
                         reactor_node->get_child(u"id"))
                         ->get();
      g_reactor.index = index;
      auto x =
          static_cast<wz::Property<int>*>(reactor_node->get_child(u"x"))->get();
      auto y =
          static_cast<wz::Property<int>*>(reactor_node->get_child(u"y"))->get();
      g_reactor.pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};
      g_reactor.ani_index = 0;
      g_reactor.ani_time = 0;

      g_reactor.f =
          static_cast<wz::Property<int>*>(reactor_node->get_child(u"f"))->get();
      auto layer = LoadReactorLayer(g_reactor.pos);

      ReactorGameInstance::data_[layer].emplace_back(g_reactor);
      index++;
    }
  }
}

}  // namespace sdlms
