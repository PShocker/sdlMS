#include "map_info_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "foothold_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <optional>
#include <ranges>
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

SDL_FRect map_info_game_instance::load_mr_border(uint32_t map_id) {
  static std::flat_map<uint32_t, SDL_FRect> cache;
  if (!cache.contains(map_id)) {
    auto map_info = load(map_id);
    if (!map_info->get_child(u"MRTop")) {
      auto fhs = foothold_game_instance::load(map_id);
      float l = fhs.begin()->second.x1;
      float t = fhs.begin()->second.y1;
      float r = fhs.begin()->second.x2;
      float b = fhs.begin()->second.y2;
      for (const auto &fh : fhs | std::views::values) {
        l = std::min({l, (float)fh.x1, (float)fh.x2});
        t = std::min({t, (float)fh.y1, (float)fh.y2});
        r = std::max({r, (float)fh.x1, (float)fh.x2});
        b = std::max({l, (float)fh.y1, (float)fh.y2});
      }
      SDL_FRect rect;
      rect.x = l;
      rect.y = t;
      rect.w = r;
      rect.h = b;
      cache[map_id] = rect;
    } else {
      SDL_FRect r;
      r.x = static_cast<wz::Property<int> *>(map_info->get_child(u"MRLeft"))
                ->get();
      r.y = static_cast<wz::Property<int> *>(map_info->get_child(u"MRTop"))
                ->get();
      r.w = static_cast<wz::Property<int> *>(map_info->get_child(u"MRRight"))
                ->get();
      r.h = static_cast<wz::Property<int> *>(map_info->get_child(u"MRBottom"))
                ->get();
      cache[map_id] = r;
    }
  }
  return cache.at(map_id);
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

uint32_t map_info_game_instance::load_return_map(uint32_t map_id) {
  uint32_t r = 0;
  auto map_info = load(map_id);
  r = static_cast<wz::Property<int> *>(map_info->get_child(u"returnMap"))
          ->get();
  return r;
}

std::u16string map_info_game_instance::load_bgm(uint32_t map_id) {
  auto map_info = load(map_id);
  if (map_info->get_child(u"bgm")) {
    auto map_bgm =
        static_cast<wz::Property<std::u16string> *>(map_info->get_child(u"bgm"))
            ->get();
    size_t pos = map_bgm.find(u'/');
    if (pos != std::u16string::npos) {
      map_bgm.insert(pos, u".img");
    }
    return map_bgm;
  }
  return u"";
}