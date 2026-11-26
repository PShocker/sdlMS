#include "src/client/game_instance/trap_game_instance.h"

#include "src/client/game/game_obj.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Property.h>
#include <wz/Wz.h>

#include <chrono>
#include <cstdint>
#include <flat_map>
#include <numeric>
#include <string>
#include <vector>

namespace sdlms {

void TrapGameInstance::Load(wz::Node* image) {
  data_ = {};
  auto map_node = image;
  uint8_t map_layer = 0;
  for (auto i : {u"0", u"1", u"2", u"3", u"4", u"5", u"6", u"7"}) {
    auto layer_node = map_node->get_child(i);
    if (layer_node == nullptr || layer_node->get_child(u"obj") == nullptr) {
      map_layer++;
      continue;
    }
    for (auto [key, val] : *layer_node->get_child(u"obj")->get_children()) {
      GameObj g_obj;
      auto obj_node = val[0];
      auto* oS = obj_node->get_child(u"oS");
      if (oS == nullptr || oS->type != wz::Type::String) {
        continue;
      }
      auto os_node = static_cast<wz::Property<std::u16string>*>(oS)->get();
      if (os_node != u"obj_trap") {
        continue;
      }
      auto l0 =
          static_cast<wz::Property<std::u16string>*>(obj_node->get_child(u"l0"))
              ->get();
      auto l1 =
          static_cast<wz::Property<std::u16string>*>(obj_node->get_child(u"l1"))
              ->get();
      auto l2 =
          static_cast<wz::Property<std::u16string>*>(obj_node->get_child(u"l2"))
              ->get();
      g_obj.path = u"Obj/" + os_node + u".img/" + l0 + u"/" + l1 + u"/" + l2;

      auto x =
          static_cast<wz::Property<int>*>(obj_node->get_child(u"x"))->get();
      auto y =
          static_cast<wz::Property<int>*>(obj_node->get_child(u"y"))->get();
      auto z =
          static_cast<wz::Property<int>*>(obj_node->get_child(u"z"))->get();

      g_obj.base = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};
      g_obj.pos = g_obj.base;

      auto flip =
          static_cast<wz::Property<int>*>(obj_node->get_child(u"f"))->get();

      g_obj.flip = flip;

      auto load_z = std::stoi(std::string{key.begin(), key.end()});

      g_obj.z = z * 100000 + load_z * 10;

      // 095 CMapLoadable::MakeVectorAnimate：向量运动宏读的是 obj 资源
      // 首帧（canvas 0）上的 moveType/moveW/moveH/moveP/moveR/centerStart。
      obj_node = WzResource::map_->find(g_obj.path);
      if (obj_node == nullptr) {
        continue;
      }
      auto read_int = [](wz::Node* frame, const char16_t* key,
                         int def) -> int {
        auto* v = frame == nullptr ? nullptr : frame->get_child(key);
        if (v == nullptr || v->type != wz::Type::Int) {
          return def;
        }
        return static_cast<wz::Property<int32_t>*>(v)->get();
      };
      // 找首帧（0 或最小数字键，需要解析 UOL/取实际 canvas）。
      wz::Node* first_frame = nullptr;
      std::vector<int> delays;
      uint8_t canvas_count = 0;
      if (obj_node->get_children() != nullptr) {
        int min_idx = -1;
        for (auto&& [k, v] : *obj_node->get_children()) {
          if (v.empty() || v[0] == nullptr) {
            continue;
          }
          auto* c = v[0];
          if (c->type == wz::Type::UOL) {
            c = static_cast<wz::Property<wz::WzUOL>*>(c)->get_uol();
          }
          if (c == nullptr) {
            continue;
          }
          // 数字帧键（obj 资源节点平铺编号）。
          bool digit = !k.empty() &&
                       k.find_first_not_of(u"0123456789") == std::u16string::npos;
          if (!digit) {
            continue;
          }
          if (c->type == wz::Type::Canvas) {
            canvas_count++;
            delays.push_back(read_int(c, u"delay", 100));
            int idx = std::stoi(std::string{k.begin(), k.end()});
            if (min_idx < 0 || idx < min_idx) {
              min_idx = idx;
              first_frame = c;
            }
          }
        }
      }
      g_obj.ani_count = canvas_count > 0 ? canvas_count : 1;
      g_obj.ani_cycle =
          static_cast<uint32_t>(std::accumulate(delays.begin(), delays.end(), 0));
      g_obj.ani_start_time = 0;

      if (first_frame != nullptr) {
        g_obj.move.type = read_int(first_frame, u"moveType", -1);
        if (g_obj.move.type >= 0) {
          // 095 默认值：moveW/moveH 100，moveP 5000。
          g_obj.move.w = read_int(first_frame, u"moveW", 100);
          g_obj.move.h = read_int(first_frame, u"moveH", 100);
          g_obj.move.p = read_int(first_frame, u"moveP", 5000);
          g_obj.move.r = read_int(first_frame, u"moveR", 0);
          g_obj.move.center_start =
              read_int(first_frame, u"centerStart", 0);
        }
      }
      // 起始相位对齐全局时钟（帧动画用周期偏移）。
      if (g_obj.ani_cycle > 0) {
        auto AnimateData = LoadAnimateIndex(delays);
        g_obj.ani_index = static_cast<uint32_t>(AnimateData.index);
        g_obj.ani_time = static_cast<uint32_t>(AnimateData.time);
      } else {
        g_obj.ani_index = 0;
        g_obj.ani_time = 0;
      }

      data_[map_layer].emplace(g_obj.z, g_obj);
    }
    map_layer++;
  }
}

TrapGameInstance::AnimateData TrapGameInstance::LoadAnimateIndex(
    std::vector<int> delays) {
  auto ds = delays;
  if (ds.empty()) {
    return {0, 0};
  }
  auto sum = std::accumulate(ds.begin(), ds.end(), 0);
  if (sum <= 0) {
    return {0, 0};
  }
  auto offset = Window::dt_time_ % sum;
  uint32_t accumulated = 0;
  uint8_t index = 0;
  for (auto j = 0; j < ds.size(); j++) {
    if (offset < accumulated + ds[j]) {
      index = j;
      break;
    }
    accumulated += ds[j];
  }
  AnimateData data;
  data.index = index;
  data.time = offset - accumulated;
  return data;
}

std::vector<int> TrapGameInstance::LoadAnimateDelays(wz::Node* node) {
  static std::flat_map<wz::Node*, std::vector<int>> cache;
  if (!cache.contains(node)) {
    for (int i = 0;; i++) {
      auto n = node->get_child(std::to_string(i));
      if (n == nullptr) {
        break;
      }
      auto* delay_node = n->get_child(u"delay");
      if (delay_node == nullptr || delay_node->type != wz::Type::Int) {
        break;
      }
      cache[node].push_back(
          static_cast<wz::Property<int32_t>*>(delay_node)->get());
    }
  }
  return cache.at(node);
}

void TrapGameInstance::Load(uint32_t map_id) {
  auto map_node = WzResource::LoadMapNode(map_id);
  if (map_node == nullptr) {
    data_ = {};
    return;
  }
  Load(map_node);
}

}  // namespace sdlms
