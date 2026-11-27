#include "src/client/system/logic/trap_logic_system.h"

#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_trap_instance.h"

#include <wz/Node.h>
#include <wz/Property.h>

#include <cmath>
#include <cstdint>
#include <numbers>

namespace sdlms {

namespace {

// 解析帧节点（含 UOL）。
wz::Node* ResolveFrame(wz::Node* obj_node, uint32_t index) {
  if (obj_node == nullptr) {
    return nullptr;
  }
  auto* frame = obj_node->get_child(std::to_string(index));
  if (frame == nullptr) {
    return nullptr;
  }
  if (frame->type == wz::Type::UOL) {
    frame = static_cast<wz::Property<wz::WzUOL>*>(frame)->get_uol();
  }
  return frame;
}

// 095 CVecProcess::MakeOscilate / MakeRotate / MakeElipticalMove 的位置
// 公式（t 为全局时钟，相位对所有客户端一致 → 陷阱位置与状态统一同步）。
// 返回相对 WZ 摆放点的偏移。
SDL_FPoint MoveOffset(const GameObj& g_trap, uint64_t now) {
  const auto& m = g_trap.move;
  if (m.type < 0 || m.p <= 0) {
    return {0.0f, 0.0f};
  }
  // 095 centerStart：相位从周期的 0（false）或半周期（true）开始。
  const double period = static_cast<double>(m.p);
  const double phase =
      (static_cast<double>(now) + (m.center_start ? period / 2 : 0.0)) / period;
  const double two_pi = 2.0 * std::numbers::pi;
  switch (m.type) {
    case 0: {
      // MakeRotate：绕原点匀速旋转（半径取 moveW）；moveR>0 时优先用
      // moveR 作为转一圈的周期。
      const double rot_period =
          m.r > 0 ? static_cast<double>(m.r) : period;
      const double a =
          two_pi * (static_cast<double>(now) / rot_period);
      const double radius = static_cast<double>(m.w);
      return {static_cast<float>(radius * std::cos(a)),
              static_cast<float>(radius * std::sin(a))};
    }
    case 1: {
      // MakeOscilate 横向：x = W * sin(2πt/P)，y 不动。
      const double a = two_pi * phase;
      return {static_cast<float>(m.w * std::sin(a)), 0.0f};
    }
    case 2: {
      // MakeOscilate 纵向：y = H * sin(2πt/P)。
      const double a = two_pi * phase;
      return {0.0f, static_cast<float>(m.h * std::sin(a))};
    }
    case 3: {
      // MakeElipticalMove 椭圆：(W·cos, H·sin)，moveP 正负决定绕行方向。
      const double a = two_pi * phase * (m.p < 0 ? -1.0 : 1.0);
      return {static_cast<float>(m.w * std::cos(a)),
              static_cast<float>(m.h * std::sin(a))};
    }
    default: {
      return {0.0f, 0.0f};
    }
  }
}

}  // namespace

void TrapLogicSystem::RunMove(GameObj& g_trap) {
  // 095：位置 = WZ 摆放点 + 运动宏在当前时刻的位移。相位用全局时钟
  //（服务器同步时间），不同客户端在同一时刻得到同一位置/状态。
  auto offset = MoveOffset(g_trap, Window::dt_time_);
  g_trap.pos = {g_trap.base.x + offset.x, g_trap.base.y + offset.y};
}

bool TrapLogicSystem::RunAttackCheck(GameObj& g_trap) {
  auto& self = CharacterGameInstance::self_;
  // 095 GetCollideObstacleRect：碰撞框取当前帧的 lt/rb（随帧动画变化，
  // 例如激光张开/收起状态各自不同的判定块）。
  auto* obj_node = WzResource::map_->find(g_trap.path);
  auto* frame = ResolveFrame(obj_node, g_trap.ani_index);
  if (frame == nullptr) {
    return false;
  }
  auto* lt = frame->get_child(u"lt");
  auto* rb = frame->get_child(u"rb");
  if (lt == nullptr || rb == nullptr || lt->type != wz::Type::Vector2D ||
      rb->type != wz::Type::Vector2D) {
    return false;
  }
  auto lt_v = static_cast<wz::Property<wz::WzVec2D>*>(lt)->get();
  auto rb_v = static_cast<wz::Property<wz::WzVec2D>*>(rb)->get();
  SDL_FRect rect{
      g_trap.pos.x + static_cast<float>(lt_v.x),
      g_trap.pos.y + static_cast<float>(lt_v.y),
      static_cast<float>(rb_v.x - lt_v.x),
      static_cast<float>(rb_v.y - lt_v.y),
  };
  return SDL_PointInRectFloat(&self.pos, &rect);
}

void TrapLogicSystem::RunCollision() {
  if (CharacterLogicSystem::self_invincible_cooldown_ >= Window::dt_now_) {
    return;
  }
  auto& self = CharacterGameInstance::self_;
  if (self.action == u"dead") {
    return;
  }
  for (auto& m : TrapGameInstance::data_) {
    for (auto& g_trap : m | std::views::values) {
      if (RunAttackCheck(g_trap)) {
        uint64_t hit_num = 1;
        CharacterLogicSystem::RunBeingHit(g_trap.pos.x, hit_num);
        fbs::AttackT at{
            .delay = Window::dt_time_,
            .num = hit_num,
            .x = self.pos.x,
            .y = self.pos.y - 30,
            // 人物受伤统一用紫色数字。
            .type = static_cast<uint8_t>(DamageData::DamageColor::kPurple),
        };
        ServerTrapInstance::HandleSAttack(0, at);
        fbs::ClientTrapAttackT cta;
        cta.map_id = SceneSystemInstance::map_id_;
        cta.payload = std::make_unique<fbs::AttackT>(at);
        ClientRequest::SendToHost(cta);
        return;
      }
    }
  }
  return;
}

bool TrapLogicSystem::Run() {
  for (auto& o : TrapGameInstance::data_) {
    for (auto& g_trap : o | std::views::values) {
      RunMove(g_trap);
      ObjLogicSystem::RunAnimate(g_trap);
      ObjLogicSystem::RunAlpha(g_trap);
    }
  }
  RunCollision();
  return true;
}

}  // namespace sdlms
