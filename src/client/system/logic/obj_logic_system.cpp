#include "src/client/system/logic/obj_logic_system.h"

#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <wz/Property.h>
#include <wz/Wz.h>

#include <cstdint>
#include <ranges>
#include <string>

namespace sdlms {

void ObjLogicSystem::RunAnimate(GameObj& g_obj) {
  if (g_obj.ani_count <= 1) {
    return;
  }
  // 陷阱（obj_trap）：帧相位按全局时钟（服务器同步时间）重算，保证
  // 多客户端的陷阱状态（激光张开/收起等）在同一时刻一致。
  // 普通地图 obj 沿用本地累加推进（095 GA_REPEAT 自驱动画，无需同步）。
  if (g_obj.ani_cycle > 0 && g_obj.move.type >= 0) {
    auto offset = Window::dt_time_ % g_obj.ani_cycle;
    g_obj.ani_time = static_cast<uint32_t>(offset);
    auto* obj_node = WzResource::map_->find(g_obj.path);
    // 累加各帧 delay 定位当前帧。
    uint32_t index = 0;
    uint32_t acc = 0;
    for (uint32_t i = 0; i < g_obj.ani_count && obj_node != nullptr; ++i) {
      auto* frame = obj_node->get_child(std::to_string(i));
      if (frame == nullptr) {
        break;
      }
      auto* delay_node = frame->get_child(u"delay");
      // obj_trap 帧经 UOL 引用时解析真实节点。
      if (delay_node == nullptr && frame->type == wz::Type::UOL) {
        frame = static_cast<wz::Property<wz::WzUOL>*>(frame)->get_uol();
        if (frame != nullptr) {
          delay_node = frame->get_child(u"delay");
        }
      }
      uint32_t delay = 100;
      if (delay_node != nullptr && delay_node->type == wz::Type::Int) {
        delay = static_cast<wz::Property<int32_t>*>(delay_node)->get();
      }
      if (offset < acc + delay) {
        index = i;
        g_obj.ani_time = offset - acc;
        break;
      }
      acc += delay;
      index = i + 1;
    }
    g_obj.ani_index = g_obj.ani_count > 0 ? index % g_obj.ani_count : 0;
    return;
  }

  g_obj.ani_time += Window::delta_time_;

  auto index = std::to_string(g_obj.ani_index);

  auto* obj_node = WzResource::map_->find(g_obj.path);
  if (obj_node == nullptr) {
    return;
  }
  obj_node = obj_node->get_child(index);
  if (obj_node == nullptr) {
    return;
  }
  if (obj_node->type == wz::Type::UOL) {
    obj_node = static_cast<wz::Property<wz::WzUOL>*>(obj_node)->get_uol();
  }
  if (obj_node == nullptr) {
    return;
  }
  int32_t delay = 100;
  auto delay_node = obj_node->get_child(u"delay");
  if (delay_node != nullptr && delay_node->type == wz::Type::Int) {
    delay = static_cast<wz::Property<int32_t>*>(delay_node)->get();
  }

  if (g_obj.ani_time >= delay) {
    g_obj.ani_time = 0;
    g_obj.ani_index += 1;
    g_obj.ani_index = g_obj.ani_index % g_obj.ani_count;
  }
}

void ObjLogicSystem::RunAlpha(GameObj& g_obj) {
  auto* obj_node = WzResource::map_->find(g_obj.path);
  if (obj_node == nullptr) {
    return;
  }
  auto index = std::to_string(g_obj.ani_index);
  obj_node = obj_node->get_child(index);
  if (obj_node == nullptr) {
    return;
  }
  if (obj_node->type == wz::Type::UOL) {
    obj_node = static_cast<wz::Property<wz::WzUOL>*>(obj_node)->get_uol();
  }
  if (obj_node == nullptr) {
    return;
  }
  if (obj_node->get_child(u"a0")) {
    int a0 = 255;
    int a1 = 255;
    a0 = static_cast<wz::Property<int>*>(obj_node->get_child(u"a0"))->get();
    if (obj_node->get_child(u"a1")) {
      a1 = static_cast<wz::Property<int>*>(obj_node->get_child(u"a1"))->get();
    }
    auto delay_node = obj_node->get_child(u"delay");
    int32_t delay = 100;
    if (delay_node != nullptr && delay_node->type == wz::Type::Int) {
      delay = static_cast<wz::Property<int32_t>*>(delay_node)->get();
    }
    if (delay <= 0) {
      return;
    }
    float t = (float)g_obj.ani_time / (float)delay;
    g_obj.alpha = a0 + (a1 - a0) * t;
  }
}

bool ObjLogicSystem::Run() {
  for (auto& o : ObjGameInstance::data_) {
    for (auto& g_obj : o | std::views::values) {
      RunAnimate(g_obj);
      RunAlpha(g_obj);
    }
  }
  return true;
}

}  // namespace sdlms
