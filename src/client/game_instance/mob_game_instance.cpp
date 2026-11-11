#include "src/client/game_instance/mob_game_instance.h"

#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {

namespace {

// 帧节点里的 delay（wz 里可能是 int 也可能是字符串）
uint32_t ReadEffectDelay(wz::Node* frame) {
  if (frame == nullptr) {
    return 100;
  }
  auto* node = frame->get_child(u"delay");
  if (node == nullptr) {
    return 100;
  }
  if (node->type == wz::Type::Int) {
    auto v = static_cast<wz::Property<int32_t>*>(node)->get();
    return v < 0 ? 0 : static_cast<uint32_t>(v);
  }
  if (node->type == wz::Type::String) {
    auto s = static_cast<wz::Property<std::u16string>*>(node)->get();
    return static_cast<uint32_t>(std::stoi(std::string{s.begin(), s.end()}));
  }
  return 100;
}

// 数字 -> u16string（帧号子节点名）
std::u16string ToU16(uint32_t value) {
  std::string narrow = std::to_string(value);
  return std::u16string(narrow.begin(), narrow.end());
}

wz::Node* FindEffectRoot(uint8_t kind, const std::u16string& path) {
  wz::Files* files = kind == 1 ? WzResource::skill_ : WzResource::mob_;
  if (files == nullptr) {
    return nullptr;
  }
  return files->find(path);
}

}  // namespace

std::u16string MobGameInstance::LoadMobName(const std::u16string& id) {
  std::u16string r = u"";
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });

  std::u16string result(view.begin(), view.end());
  auto str_node = WzResource::string_->find(u"Mob.img/" + result + u"/name");
  auto str = static_cast<wz::Property<std::u16string>*>(str_node)->get();
  return str;
}

wz::Node* MobGameInstance::LoadMobInfo(const std::u16string& id) {
  static std::flat_map<std::u16string, wz::Node*> cache;
  if (!cache.contains(id)) {
    auto mob_node = LoadLinkMobNode(id);
    cache[id] = mob_node->get_child(u"info");
  }
  return cache.at(id);
}

wz::Node* MobGameInstance::LoadLinkMobNode(const std::u16string& id) {
  auto mob_node = WzResource::mob_->find(id + u".img");
  while (mob_node->find(u"info/link")) {
    auto link_node = mob_node->find(u"info/link");
    auto link = static_cast<wz::Property<std::u16string>*>(link_node)->get();
    mob_node = WzResource::mob_->find(link + u".img");
  }
  return mob_node;
}

void MobGameInstance::Load(uint32_t map_id) {
  data_ = {};
  effect_.clear();
}

void MobGameInstance::AddEffect(uint32_t mob_index, uint8_t kind,
                                const std::u16string& path, float x, float y,
                                float x2, float y2, uint64_t move_ms, bool flip,
                                uint64_t delay) {
  if (path.empty()) {
    return;
  }
  GameMobEffect effect;
  effect.mob_index = mob_index;
  effect.kind = kind;
  effect.path = path;
  effect.flip = flip;
  effect.move_ms = move_ms;
  effect.loop = move_ms > 0;
  effect.mx = x;
  effect.my = y;
  effect.mx2 = x2;
  effect.my2 = y2;
  // 静态特效：世界坐标 -> 相对怪物原点的偏移（跟随怪物移动）
  float mob_x = x;
  float mob_y = y;
  auto it = data_.find(static_cast<int32_t>(mob_index));
  if (it != data_.end()) {
    mob_x = it->second.mob.pos.x;
    mob_y = it->second.mob.pos.y;
  }
  effect.ox = x - mob_x;
  effect.oy = y - mob_y;
  effect.start = Window::dt_now_ + delay;
  effect.frame_at = effect.start;
  effect_.push_back(std::move(effect));
}

wz::Node* MobGameInstance::LoadEffectFrameNode(const GameMobEffect& effect,
                                               uint32_t index) {
  auto* root = FindEffectRoot(effect.kind, effect.path);
  if (root == nullptr || root->get_children() == nullptr) {
    return nullptr;
  }
  return root->get_child(ToU16(index));
}

uint32_t MobGameInstance::LoadEffectFrameDelay(wz::Node* frame) {
  return ReadEffectDelay(frame);
}

void MobGameInstance::RunEffect() {
  const auto now = Window::dt_now_;
  for (auto it = effect_.begin(); it != effect_.end();) {
    auto& effect = *it;
    if (effect.done) {
      it = effect_.erase(it);
      continue;
    }
    if (now < effect.start) {
      ++it;
      continue;
    }
    // 飞行体：到达终点即销毁
    if (effect.move_ms > 0 && now - effect.start >= effect.move_ms) {
      it = effect_.erase(it);
      continue;
    }
    // 懒加载帧延时表
    if (effect.delays.empty()) {
      for (uint32_t i = 0; i < 64; ++i) {
        auto* frame = LoadEffectFrameNode(effect, i);
        if (frame == nullptr) {
          break;
        }
        effect.delays.push_back(LoadEffectFrameDelay(frame));
      }
      if (effect.delays.empty()) {
        effect.done = true;
        continue;
      }
      // wz 里偶有 0ms 帧，避免死循环
      for (auto& d : effect.delays) {
        if (d == 0) {
          d = 1;
        }
      }
      effect.frame_at = effect.start;
    }
    // 按帧延时推进
    while (now - effect.frame_at >= effect.delays[effect.index]) {
      effect.frame_at += effect.delays[effect.index];
      ++effect.index;
      if (effect.index >= effect.delays.size()) {
        if (effect.loop) {
          effect.index = 0;
          continue;
        }
        effect.index = static_cast<uint32_t>(effect.delays.size() - 1);
        effect.done = true;
        break;
      }
    }
    ++it;
  }
}

}  // namespace sdlms
