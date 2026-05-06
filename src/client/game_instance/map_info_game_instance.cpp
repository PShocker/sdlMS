#include "map_info_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <string>

wz::Node *map_info_game_instance::load(uint32_t map_id) {
  auto map_node = wz_resource::load_map_node(map_id);
  auto map_info = map_node->get_child(u"info");
  return map_info;
}

SDL_FRect map_info_game_instance::load_vr_border(uint32_t map_id) {
  SDL_FRect r;
  auto map_info = load(map_id);
  r.x = static_cast<wz::Property<int> *>(map_info->get_child(u"VRTop"))->get();
  r.y = static_cast<wz::Property<int> *>(map_info->get_child(u"VRLeft"))->get();
  r.w =
      static_cast<wz::Property<int> *>(map_info->get_child(u"VRBottom"))->get();
  r.h =
      static_cast<wz::Property<int> *>(map_info->get_child(u"VRRight"))->get();
  return r;
}

std::optional<SDL_FRect>
map_info_game_instance::load_swim_rect(uint32_t map_id) {
  static std::flat_map<uint32_t, std::optional<SDL_FRect>> cache;
  if (!cache.contains(map_id)) {
    std::optional<SDL_FRect> swim_r;
    auto map_info = load(map_id);

    cache[map_id] = swim_r;
  }
  return cache.at(map_id);
}

std::vector<SDL_FPoint> map_info_game_instance::load_seat(uint32_t map_id) {
  static std::flat_map<uint32_t, std::vector<SDL_FPoint>> cache;
  if (!cache.contains(map_id)) {
    std::vector<SDL_FPoint> vec;
    auto map_info = load(map_id);
    if (map_info->get_child(u"seat")) {
      for (auto [k, v] : *map_info->get_child(u"seat")) {
        auto seat_pos = static_cast<wz::Property<wz::WzVec2D> *>(v[0])->get();
        vec.push_back(SDL_FPoint{static_cast<float>(seat_pos.x),
                                 static_cast<float>(seat_pos.y)});
      }
    }
    cache[map_id] = vec;
  }
  return cache.at(map_id);
}