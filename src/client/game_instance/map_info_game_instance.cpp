#include "src/client/game_instance/map_info_game_instance.h"

#include "src/client/game_instance/foothold_game_instance.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <flat_map>
#include <optional>
#include <ranges>
#include <string>

namespace sdlms {

wz::Node* MapInfoGameInstance::Load(uint32_t map_id) {
  auto map_node = WzResource::LoadMapNode(map_id);
  auto map_info = map_node->get_child(u"info");
  return map_info;
}

SDL_FRect MapInfoGameInstance::LoadVrBorder(uint32_t map_id) {
  SDL_FRect r;
  auto map_info = Load(map_id);
  r.x = static_cast<wz::Property<int>*>(map_info->get_child(u"VRTop"))->get();
  r.y = static_cast<wz::Property<int>*>(map_info->get_child(u"VRLeft"))->get();
  r.w =
      static_cast<wz::Property<int>*>(map_info->get_child(u"VRBottom"))->get();
  r.h = static_cast<wz::Property<int>*>(map_info->get_child(u"VRRight"))->get();
  return r;
}

SDL_FRect MapInfoGameInstance::LoadMrBorder(uint32_t map_id) {
  static std::flat_map<uint32_t, SDL_FRect> cache;
  if (!cache.contains(map_id)) {
    auto map_info = Load(map_id);
    if (!map_info->get_child(u"MRTop")) {
      auto fhs = FootholdGameInstance::Load(map_id);
      if (fhs.empty()) {
        cache[map_id] = SDL_FRect{};
        return cache.at(map_id);
      }
      float l = fhs.begin()->second.x1;
      float t = fhs.begin()->second.y1;
      float r = fhs.begin()->second.x2;
      float b = fhs.begin()->second.y2;
      for (const auto& fh : fhs | std::views::values) {
        l = std::min({l, (float)fh.x1, (float)fh.x2});
        t = std::min({t, (float)fh.y1, (float)fh.y2});
        r = std::max({r, (float)fh.x1, (float)fh.x2});
        b = std::max({b, (float)fh.y1, (float)fh.y2});
      }
      SDL_FRect rect;
      rect.x = l;
      rect.y = t;
      rect.w = r;
      rect.h = b;
      cache[map_id] = rect;
    } else {
      SDL_FRect r;
      r.x = static_cast<wz::Property<int>*>(map_info->get_child(u"MRLeft"))
                ->get();
      r.y =
          static_cast<wz::Property<int>*>(map_info->get_child(u"MRTop"))->get();
      r.w = static_cast<wz::Property<int>*>(map_info->get_child(u"MRRight"))
                ->get();
      r.h = static_cast<wz::Property<int>*>(map_info->get_child(u"MRBottom"))
                ->get();
      cache[map_id] = r;
    }
  }
  return cache.at(map_id);
}

std::optional<SDL_FRect> MapInfoGameInstance::LoadSwimRect(uint32_t map_id) {
  static std::flat_map<uint32_t, std::optional<SDL_FRect>> cache;
  if (!cache.contains(map_id)) {
    std::optional<SDL_FRect> swim_r;
    auto* map_info = Load(map_id);
    auto* node = map_info->get_child(u"swim");
    bool swim = node != nullptr && node->type == wz::Type::Int &&
                static_cast<wz::Property<int>*>(node)->get() != 0;
    if (swim) {
      // 095 RestoreSwinArea 优先用 swimArea 矩形；本数据包没有该节点，
      // 游泳区退回整张地图的 VR 边界（095 对无 swimArea 的地图同样按 VR）。
      auto load = [map_info](const char16_t* name, float fallback) {
        auto* child = map_info->get_child(name);
        if (child == nullptr || child->type != wz::Type::Int) {
          return fallback;
        }
        return static_cast<float>(static_cast<wz::Property<int>*>(child)->get());
      };
      auto left = load(u"VRLeft", 0.0f);
      auto top = load(u"VRTop", 0.0f);
      auto right = load(u"VRRight", 0.0f);
      auto bottom = load(u"VRBottom", 0.0f);
      if (right > left && bottom > top) {
        swim_r = SDL_FRect{left, top, right - left, bottom - top};
      }
    }
    cache[map_id] = swim_r;
  }
  return cache.at(map_id);
}

bool MapInfoGameInstance::IsSwimMap(uint32_t map_id) {
  return LoadSwimRect(map_id).has_value();
}

bool MapInfoGameInstance::IsFlyingMap(uint32_t map_id) {
  static std::map<uint32_t, bool> cache;
  if (!cache.contains(map_id)) {
    bool fly = false;
    auto* map_info = Load(map_id);
    auto* node = map_info->get_child(u"fly");
    if (node != nullptr && node->type == wz::Type::Int) {
      fly = static_cast<wz::Property<int>*>(node)->get() != 0;
    }
    cache[map_id] = fly;
  }
  return cache.at(map_id);
}

bool MapInfoGameInstance::IsSwimming(uint32_t map_id, SDL_FPoint pos) {
  auto rect = LoadSwimRect(map_id);
  return rect.has_value() && SDL_PointInRectFloat(&pos, &rect.value());
}

std::vector<SDL_FPoint> MapInfoGameInstance::LoadSeat(uint32_t map_id) {
  static std::flat_map<uint32_t, std::vector<SDL_FPoint>> cache;
  if (!cache.contains(map_id)) {
    std::vector<SDL_FPoint> vec;
    auto map_info = Load(map_id);
    if (map_info->get_child(u"seat")) {
      for (auto [k, v] : *map_info->get_child(u"seat")) {
        auto seat_pos = static_cast<wz::Property<wz::WzVec2D>*>(v[0])->get();
        vec.push_back(SDL_FPoint{static_cast<float>(seat_pos.x),
                                 static_cast<float>(seat_pos.y)});
      }
    }
    cache[map_id] = vec;
  }
  return cache.at(map_id);
}

uint32_t MapInfoGameInstance::LoadReturnMap(uint32_t map_id) {
  uint32_t r = 0;
  auto map_info = Load(map_id);
  r = static_cast<wz::Property<int>*>(map_info->get_child(u"returnMap"))->get();
  return r;
}

int32_t MapInfoGameInstance::LoadDecHP(uint32_t map_id) {
  auto* info = Load(map_id);
  if (info == nullptr) {
    return 0;
  }
  auto* node = info->get_child(u"decHP");
  if (node == nullptr || node->type != wz::Type::Int) {
    return 0;
  }
  return static_cast<wz::Property<int32_t>*>(node)->get();
}

uint32_t MapInfoGameInstance::LoadTimeLimit(uint32_t map_id) {
  auto* info = Load(map_id);
  if (info == nullptr) {
    return 0;
  }
  auto* node = info->get_child(u"timeLimit");
  if (node == nullptr || node->type != wz::Type::Int) {
    return 0;
  }
  return static_cast<uint32_t>(
      std::max(0, static_cast<wz::Property<int32_t>*>(node)->get()));
}

uint32_t MapInfoGameInstance::LoadForcedReturn(uint32_t map_id) {
  auto* info = Load(map_id);
  if (info == nullptr) {
    return 0;
  }
  auto* node = info->get_child(u"forcedReturn");
  if (node == nullptr || node->type != wz::Type::Int) {
    return 0;
  }
  return static_cast<uint32_t>(
      std::max(0, static_cast<wz::Property<int32_t>*>(node)->get()));
}

double MapInfoGameInstance::LoadRecovery(uint32_t map_id) {
  auto map_info = Load(map_id);
  if (map_info == nullptr) {
    return 1.0;
  }
  auto* node = map_info->get_child(u"recovery");
  if (node == nullptr) {
    return 1.0;
  }
  if (node->type == wz::Type::Double) {
    return static_cast<wz::Property<double>*>(node)->get();
  }
  if (node->type == wz::Type::Int) {
    return static_cast<double>(static_cast<wz::Property<int>*>(node)->get());
  }
  return 1.0;
}

std::vector<SDL_FRect> MapInfoGameInstance::LoadAreas(uint32_t map_id) {
  static std::flat_map<uint32_t, std::vector<SDL_FRect>> cache;
  if (!cache.contains(map_id)) {
    std::vector<SDL_FRect> areas;
    auto* map_node = WzResource::LoadMapNode(map_id);
    auto* node = map_node != nullptr ? map_node->get_child(u"area") : nullptr;
    if (node != nullptr) {
      for (auto [key, values] : node->children) {
        for (auto* area : values) {
          auto load_int = [area](const char16_t* name) {
            auto* child = area->get_child(name);
            if (child == nullptr || child->type != wz::Type::Int) {
              return 0;
            }
            return static_cast<wz::Property<int>*>(child)->get();
          };
          auto x1 = load_int(u"x1");
          auto x2 = load_int(u"x2");
          auto y1 = load_int(u"y1");
          auto y2 = load_int(u"y2");
          areas.push_back(SDL_FRect{
              static_cast<float>(std::min(x1, x2)),
              static_cast<float>(std::min(y1, y2)),
              static_cast<float>(std::abs(x2 - x1)),
              static_cast<float>(std::abs(y2 - y1)),
          });
        }
      }
    }
    cache[map_id] = std::move(areas);
  }
  return cache.at(map_id);
}

bool MapInfoGameInstance::IsInArea(uint32_t map_id, float x, float y) {
  SDL_FPoint p{x, y};
  for (const auto& area : LoadAreas(map_id)) {
    if (SDL_PointInRectFloat(&p, &area)) {
      return true;
    }
  }
  return false;
}

std::optional<MapInfoGameInstance::CoconutEventInfo>
MapInfoGameInstance::LoadCoconutEvent(uint32_t map_id) {
  static std::flat_map<uint32_t, std::optional<CoconutEventInfo>> cache;
  if (!cache.contains(map_id)) {
    std::optional<CoconutEventInfo> info;
    auto* map_node = WzResource::LoadMapNode(map_id);
    auto* node = map_node != nullptr ? map_node->get_child(u"coconut") : nullptr;
    if (node != nullptr) {
      CoconutEventInfo event;
      auto load_int = [node](const char16_t* name) {
        auto* child = node->get_child(name);
        if (child == nullptr || child->type != wz::Type::Int) {
          return 0;
        }
        return static_cast<wz::Property<int>*>(child)->get();
      };
      auto load_str = [node](const char16_t* name) {
        auto* child = node->get_child(name);
        if (child == nullptr || child->type != wz::Type::String) {
          return std::u16string{};
        }
        return static_cast<wz::Property<std::u16string>*>(child)->get();
      };
      event.event_name = load_str(u"eventName");
      event.object_name = load_str(u"eventObjectName");
      event.time_default = load_int(u"timeDefault");
      event.time_expand = load_int(u"timeExpand");
      event.time_finish = load_int(u"timeFinish");
      event.time_message = load_int(u"timeMessage");
      info = std::move(event);
    }
    cache[map_id] = std::move(info);
  }
  return cache.at(map_id);
}

int32_t MapInfoGameInstance::LoadFieldLimit(uint32_t map_id) {
  static std::flat_map<uint32_t, int32_t> cache;
  if (!cache.contains(map_id)) {
    int32_t limit = 0;
    auto* map_info = Load(map_id);
    if (auto* node = map_info->get_child(u"fieldLimit");
        node != nullptr && node->type == wz::Type::Int) {
      limit = static_cast<wz::Property<int>*>(node)->get();
    }
    cache[map_id] = limit;
  }
  return cache.at(map_id);
}

bool MapInfoGameInstance::IsSkillDisabled(uint32_t map_id) {
  // 095 CField::IsUnableToUseSkill：fieldLimit bit1。
  return (LoadFieldLimit(map_id) & 0x2) != 0;
}

bool MapInfoGameInstance::IsShopDisabled(uint32_t map_id) {
  // 095 SendMigrateToShopRequest：fieldLimit bit4。
  return (LoadFieldLimit(map_id) & 0x10) != 0;
}

bool MapInfoGameInstance::IsMinigameDisabled(uint32_t map_id) {
  // 095 SendCreateMiniGameRequest：fieldLimit bit7（SLOBYTE(option) < 0）。
  return (LoadFieldLimit(map_id) & 0x80) != 0;
}

bool MapInfoGameInstance::IsPortalScrollDisabled(uint32_t map_id) {
  // 095 SendPortalScrollUseRequest：fieldLimit bit8。
  return (LoadFieldLimit(map_id) & 0x100) != 0;
}

bool MapInfoGameInstance::IsCashItemDisabled(uint32_t map_id) {
  // 095 SendConsumeCashItemUseRequest：fieldLimit bit18。
  return (LoadFieldLimit(map_id) & 0x40000) != 0;
}

std::u16string MapInfoGameInstance::LoadBgm(uint32_t map_id) {
  auto map_info = Load(map_id);
  if (map_info->get_child(u"bgm")) {
    auto map_bgm =
        static_cast<wz::Property<std::u16string>*>(map_info->get_child(u"bgm"))
            ->get();
    size_t pos = map_bgm.find(u'/');
    if (pos != std::u16string::npos) {
      map_bgm.insert(pos, u".img");
    }
    return map_bgm;
  }
  return u"";
}

}  // namespace sdlms
