#include "src/server/server_system/server_mob_system.h"

#include "src/client/game/game_debuff.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/physic/physic.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_instance/server_scene_instance.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <flat_map>
#include <memory>
#include <random>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace sdlms {

namespace {

// 移动 / 动作
constexpr uint32_t kMinFrameIntervalMs = 33;
constexpr float kJumpSpeed = -555.0f;
constexpr float kFallVspeedMin = -5550.0f;
constexpr float kFallVspeedMax = 670.0f;
constexpr float kFlyDeadZone = 10.0f;     // 飞行到点死区
constexpr float kChaseDeadZone = 100.0f;  // 追击贴脸死区
constexpr uint32_t kWanderMinMs = 330;    // 巡游换向间隔
constexpr uint32_t kWanderMaxMs = 990;

// 仇恨 / 攻击
constexpr float kAggroRange = 280.0f;   // 主动仇恨水平距离
constexpr float kAggroHeight = 120.0f;  // 主动仇恨高度差
constexpr float kLoseRange = 900.0f;    // 放弃追击水平距离
constexpr float kLoseHeight = 400.0f;   // 放弃追击高度差
constexpr uint32_t kChaseMs = 180'000;  // 追击持续时间（v95: 180~240s）
constexpr uint32_t kAttackGapMs = 300;  // 动作结束后的攻击间隔

// 被打断位移（v95 CVecCtrlMob::UpdateActiveInterrupted）
constexpr uint32_t kInterruptSliceMs = 30;  // 每片最多 30ms
constexpr float kMobMass = 100.0f;          // CAttrShoe::mass 默认值
constexpr float kSlowDiv = 3.0f;            // 减速档：每片速度 /3
constexpr float kStopNear = 10.0f;          // 5/7/6/8/9 档的停止距离
constexpr float kSlowNear = 40.0f;          // 5/7 档的减速距离
constexpr float kSlowNearFar = 100.0f;      // 6/8/9 档的减速距离
constexpr float kAccessPad = 40.0f;         // 访问限位外的减速余量

// 玩家身体矩形（宽 36 高 60，脚在 pos.y）
constexpr float kPlayerHalfWidth = 18.0f;
constexpr float kPlayerHeight = 60.0f;

int8_t Sign(float value) {
  if (value > 0) {
    return 1;
  }
  if (value < 0) {
    return -1;
  }
  return 0;
}

// v95 ReturnDecSpeed(v, f, m, 0, tSec)：朝 0 减速
float DecSpeed(float v, float decel, float dt) {
  if (v > 0.0f) {
    return std::max(0.0f, v - decel * dt);
  }
  if (v < 0.0f) {
    return std::min(0.0f, v + decel * dt);
  }
  return 0.0f;
}

// foothold 的段长与单位向量；起点统一归一化到左端（v95 要求 m_x1 < m_x2）
struct FootholdAxis {
  float ox = 0.0f;
  float oy = 0.0f;
  float len = 0.0f;
  float uvx = 0.0f;
  float uvy = 0.0f;
  bool valid = false;
};

FootholdAxis LoadAxis(const GameFoothold& fh) {
  FootholdAxis axis;
  float dx = static_cast<float>(fh.x2 - fh.x1);
  float dy = static_cast<float>(fh.y2 - fh.y1);
  const bool forward = dx > 0.0f || (dx == 0.0f && dy >= 0.0f);
  if (!forward) {
    dx = -dx;
    dy = -dy;
  }
  axis.len = std::hypot(dx, dy);
  if (axis.len <= 0.0f) {
    return axis;
  }
  axis.ox = static_cast<float>(forward ? fh.x1 : fh.x2);
  axis.oy = static_cast<float>(forward ? fh.y1 : fh.y2);
  axis.uvx = dx / axis.len;
  axis.uvy = dy / axis.len;
  axis.valid = true;
  return axis;
}

}  // namespace

std::u16string ServerMobSystem::LoadDefaultAction(const ServerMob& mob) {
  if (mob.move_ability == 4) {
    return ServerMobInstance::LoadFineAction(mob.id, u"fly");
  }
  return ServerMobInstance::LoadFineAction(mob.id, u"stand");
}

void ServerMobSystem::RunSetAction(ServerMob& mob,
                                   const std::u16string& action) {
  if (mob.action == action) {
    return;
  }
  mob.action = action;
  mob.one_shot = false;
  mob.action_end = 0;
  mob.attack_hit = 0;
}

void ServerMobSystem::RunMoveAction(ServerMob& mob) {
  switch (mob.move_ability) {
    case 4: {  // 飞行：无地形约束，用 fly 动作
      if (mob.input_x != 0) {
        mob.flip = mob.input_x > 0;
      }
      RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"fly"));
      return;
    }
    case 0: {  // 原地怪：只转向
      if (mob.input_x != 0) {
        mob.flip = mob.input_x > 0;
      }
      RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"stand"));
      mob.hspeed = 0;
      return;
    }
    default: {  // 1 / 3 / 6：地面行走
      if (mob.fh == 0) {
        RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"jump"));
        return;
      }
      if (mob.input_x != 0) {
        mob.flip = mob.input_x > 0;
        RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"move"));
        mob.hspeed = mob.input_x > 0 ? mob.hspeed_max : -mob.hspeed_max;
      } else {
        RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"stand"));
        mob.hspeed = 0;
      }
      return;
    }
  }
}

bool ServerMobSystem::CanAttack(const ServerMob& mob) {
  if (ServerMobInstance::LoadMobInfoInt(mob.id, u"notAttack", 0) != 0) {
    return false;
  }
  return !ServerMobInstance::LoadMobAttacks(mob.id).empty();
}

SDL_FRect ServerMobSystem::LoadTargetRect(uint64_t client_id) {
  SDL_FRect rect{0, 0, 0, 0};
  auto& clients = ServerClientInstance::clients_;
  if (!clients.contains(client_id) ||
      clients.at(client_id).player_t.character == nullptr) {
    return rect;
  }
  const auto& state = *clients.at(client_id).player_t.character->state;
  rect.x = state.x - kPlayerHalfWidth;
  rect.y = state.y - kPlayerHeight;
  rect.w = kPlayerHalfWidth * 2;
  rect.h = kPlayerHeight;
  return rect;
}

void ServerMobSystem::RunTarget(ServerMob& mob) {
  const auto now = Window::dt_now_;
  auto& clients = ServerClientInstance::clients_;

  // 1) 目标失效检查
  if (mob.target_id != 0) {
    bool valid =
        clients.contains(mob.target_id) &&
        clients.at(mob.target_id).player_t.character != nullptr;
    if (valid) {
      const auto& state = *clients.at(mob.target_id).player_t.character->state;
      float dx = state.x - mob.pos.x;
      float dy = state.y - mob.pos.y;
      if (state.action == "dead" || std::fabs(dx) > kLoseRange ||
          std::fabs(dy) > kLoseHeight || now >= mob.chase_end) {
        valid = false;
      }
    }
    if (!valid) {
      mob.target_id = 0;
    }
  }
  if (mob.target_id != 0) {
    return;
  }

  // 2) 不会攻击的怪不建立仇恨
  if (!CanAttack(mob)) {
    return;
  }

  // 3) 找最近的可攻击玩家（v95 TryFirstAttack）
  uint64_t best_id = 0;
  float best_dist = kAggroRange;
  for (const auto& [client_id, client] : clients) {
    if (client.map_id != map_id_ || client.player_t.character == nullptr) {
      continue;
    }
    const auto& state = *client.player_t.character->state;
    if (state.action == "dead") {
      continue;
    }
    float dx = std::fabs(state.x - mob.pos.x);
    float dy = std::fabs(state.y - mob.pos.y);
    if (dy > kAggroHeight) {
      continue;
    }
    if (dx < best_dist) {
      best_dist = dx;
      best_id = client_id;
    }
  }
  if (best_id != 0) {
    mob.target_id = best_id;
    mob.chase_end = now + kChaseMs;
  }
}

void ServerMobSystem::RunInput(ServerMob& mob) {
  const auto now = Window::dt_now_;
  if (mob.move_ability == 0 || mob.move_ability == 4) {
    // 原地怪没有水平输入；飞行怪的方向在 RunFly 里处理
    mob.input_x = 0;
    return;
  }

  auto& clients = ServerClientInstance::clients_;
  if (mob.target_id != 0 && clients.contains(mob.target_id) &&
      clients.at(mob.target_id).player_t.character != nullptr) {
    const auto& state = *clients.at(mob.target_id).player_t.character->state;
    float dx = state.x - mob.pos.x;
    // 贴脸死区：|dx| < 100 时保持当前方向（v95 CtrlUpdateActiveMove）
    if (std::fabs(dx) >= kChaseDeadZone || mob.input_x == 0) {
      mob.input_x = dx >= 0 ? 1 : -1;
    }
    return;
  }

  // 无目标：每 330~990ms 随机掷一次方向（-1 / 0 / 1），窗口内保持不变
  if (now < mob.think) {
    return;
  }
  std::uniform_int_distribution<int32_t> dist(-1, 1);
  mob.input_x = Sign(static_cast<float>(dist(RandomGameInstance::gen_)));
  std::uniform_int_distribution<uint32_t> dist2(kWanderMinMs, kWanderMaxMs);
  mob.think = now + dist2(RandomGameInstance::gen_);
}

bool ServerMobSystem::RunAttackRange(const ServerMob& mob,
                                     const MobAttack& attack,
                                     const SDL_FRect& target) {
  // flip == true 表示朝右；素材默认朝左
  const float facing = mob.flip ? 1.0f : -1.0f;
  switch (attack.type) {
    case MobAttack::RangeType::kRect:
    case MobAttack::RangeType::kArea: {
      float cell_width = std::fabs(attack.rect.w);
      float left = attack.rect.x;
      float right = attack.rect.x + cell_width;
      if (attack.type == MobAttack::RangeType::kArea && attack.area_count > 0) {
        // 区域攻击覆盖 [start, start+areaCount) 这一段格子
        left = attack.rect.x + cell_width * static_cast<float>(attack.start);
        right = attack.rect.x +
                cell_width * static_cast<float>(attack.start + attack.area_count);
      }
      SDL_FRect rect;
      rect.w = std::fabs(right - left);
      rect.h = std::fabs(attack.rect.h);
      rect.y = mob.pos.y + std::min(attack.rect.y, attack.rect.y + attack.rect.h);
      if (mob.flip) {
        rect.x = mob.pos.x - right;
      } else {
        rect.x = mob.pos.x + left;
      }
      return SDL_HasRectIntersectionFloat(&rect, &target);
    }
    case MobAttack::RangeType::kTrapezoid: {
      // v95 IsRectIntersectWithTrapezoid：从攻击原点沿朝向切片，
      // 纵向半高 = 水平距离 / 4
      float origin_x = mob.pos.x + facing * std::fabs(attack.sp.x);
      float origin_y = mob.pos.y + attack.sp.y;
      float center_x = target.x + target.w / 2;
      float center_y = target.y + target.h / 2;
      float dx = (center_x - origin_x) * facing;  // 朝向前方为正
      float dy = std::fabs(center_y - origin_y);
      if (dx < -target.w / 2 || dx > static_cast<float>(attack.range)) {
        return false;
      }
      float half = std::max(1.0f, std::fabs(dx) / 4.0f) + 1.0f;
      return dy - target.h / 2 <= half;
    }
    default: {
      return false;
    }
  }
}

bool ServerMobSystem::RunTryAttack(ServerMob& mob) {
  if (mob.target_id == 0 || Window::dt_now_ < mob.attack_cd) {
    return false;
  }
  const auto& attacks = ServerMobInstance::LoadMobAttacks(mob.id);
  if (attacks.empty()) {
    return false;
  }
  auto& clients = ServerClientInstance::clients_;
  if (!clients.contains(mob.target_id) ||
      clients.at(mob.target_id).player_t.character == nullptr) {
    return false;
  }

  const auto& state = *clients.at(mob.target_id).player_t.character->state;
  mob.flip = state.x > mob.pos.x;

  auto target = LoadTargetRect(mob.target_id);
  std::vector<size_t> candidates;
  for (size_t i = 0; i < attacks.size(); ++i) {
    if (attacks[i].inactive) {
      continue;
    }
    if (!RunAttackRange(mob, attacks[i], target)) {
      continue;
    }
    candidates.push_back(i);
  }
  if (candidates.empty()) {
    return false;
  }

  // v95 IsTargetInAttackRange：候选之间等概率随机
  std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
  size_t index = candidates[dist(RandomGameInstance::gen_)];
  const auto& attack = attacks[index];

  auto delay = ServerMobInstance::LoadActionDelay(mob.id, attack.action);
  if (delay == 0) {
    delay = 500;
  }
  // 判定时刻 = 动作开始 + attackAfter（v95 用毫秒而不是帧号）
  uint64_t hit_after = attack.attack_after;
  if (hit_after == 0) {
    hit_after = delay * 2 / 3;
  }

  const auto now = Window::dt_now_;
  mob.action = attack.action;
  mob.one_shot = true;
  // attackAfter 可能晚于动画结束（v95 里两者独立），此处保证动作覆盖到判定时刻
  mob.action_end = now + std::max(delay, hit_after + 100);
  mob.attack_hit = now + hit_after;
  mob.attack_index = static_cast<int32_t>(index);
  mob.attack_cd = now + delay + kAttackGapMs;
  mob.input_x = 0;
  mob.hspeed = 0;
  mob.hforce = 0;
  return true;
}

void ServerMobSystem::RunAttackHit(ServerMob& mob) {
  mob.attack_hit = 0;
  const auto& attacks = ServerMobInstance::LoadMobAttacks(mob.id);
  if (mob.attack_index < 0 ||
      static_cast<size_t>(mob.attack_index) >= attacks.size()) {
    return;
  }
  if (mob.target_id == 0 ||
      !ServerClientInstance::clients_.contains(mob.target_id)) {
    return;
  }
  const auto& attack = attacks[static_cast<size_t>(mob.attack_index)];

  // 判定时刻重新取一次目标位置：目标已经离开攻击范围就落空
  // （v95 ProcessAttack 在 tAttackAfter 时才做 IntersectRect）
  auto target = LoadTargetRect(mob.target_id);
  if (target.w <= 0 || !RunAttackRange(mob, attack, target)) {
    return;
  }

  // 攻击力 = 怪物 PADamage + 该攻击的 PADamage，伤害浮动 85%~100%
  int32_t pad = ServerMobInstance::LoadMobInfoInt(mob.id, u"PADamage", 0);
  pad = std::clamp(pad + attack.pad, 0, 29999);
  if (pad <= 0) {
    pad = 1;
  }
  std::uniform_real_distribution<float> dist(0.85f, 1.0f);
  auto damage = static_cast<uint64_t>(
      std::max(1.0f, static_cast<float>(pad) * dist(RandomGameInstance::gen_)));

  fbs::ServerMobAttackPlayerT event;
  event.mob_index = mob.index;
  event.client_id = mob.target_id;
  event.payload = std::make_unique<fbs::AttackT>(fbs::AttackT{
      .delay = Window::dt_time_,
      .num = damage,
      .x = mob.pos.x,
      .y = mob.pos.y,
      .type = 0,
  });

  fbs::MobEventUnionUnion muu;
  muu.Set(std::move(event));
  events_.payload.push_back(std::move(muu));
}

void ServerMobSystem::RunInterruptStart(ServerMob& mob, int32_t type,
                                        float end_x, float end_y,
                                        uint32_t t_elapse) {
  auto& it = mob.interrupt;
  it.type = type;
  it.end_x = end_x;
  it.end_y = end_y;
  it.vy = 0.0f;
  it.drag = 0.0f;
  it.access = 0.0f;
  it.relocated = false;

  // v95 GenerateMovePath 里的 nMoveType -> (初速, 阻力, 访问限位) 表
  float vx = 0.0f;
  const float adx = std::fabs(mob.pos.x - end_x);
  switch (type) {
    case 2: {
      vx = 300.0f;
      it.drag = 20000.0f;
      break;
    }
    case 3: {
      it.access = ServerMobInstance::LoadMobHalfWidth(mob.id) + 45.0f;
      vx = adx < it.access ? 0.0f : -1000.0f;
      break;
    }
    case 4: {
      it.access = 75.0f;
      vx = adx < 75.0f ? 0.0f : -1400.0f;
      break;
    }
    case 5: {
      vx = adx >= 100.0f ? 1000.0f : 300.0f;
      break;
    }
    case 6: {
      vx = adx >= 100.0f ? 700.0f : 300.0f;
      break;
    }
    case 7: {
      vx = 300.0f;
      break;
    }
    case 8: {
      vx = adx >= 100.0f ? -700.0f : -300.0f;
      break;
    }
    case 9: {
      vx = 700.0f;
      break;
    }
    case 10:
    case 12:
    case 13: {
      vx = 0.0f;
      break;
    }
    default: {  // 0 / 1 / 11 / 14 ...
      vx = 130.0f;
      it.drag = 40000.0f;
      break;
    }
  }

  // 方向：3~9 是"朝终点推/拉"，其余是"背向朝向被弹开"
  // （095: bDirLeft = (bLeft == 0)，随后 vx = -vx，符号完全由方向位决定）
  float dir = 0.0f;
  const bool towards_target = type >= 3 && type <= 9;
  if (towards_target && adx > 1.0f) {
    dir = end_x > mob.pos.x ? 1.0f : -1.0f;
  } else {
    dir = mob.flip ? -1.0f : 1.0f;
  }
  // 3~9 档表里给的是带符号的初速，真正方向由 dir 决定
  it.vx = (towards_target ? std::fabs(vx) : vx) * dir;
  it.active = true;
  it.end_time = Window::dt_now_ + t_elapse;
}

void ServerMobSystem::RunInterrupt(ServerMob& mob, int32_t type, float end_x,
                                   float end_y) {
  // 095 里 tElapse 传的是当前一次性动作的总时长
  auto t_elapse = ServerMobInstance::LoadActionDelay(mob.id, mob.action);
  if (t_elapse == 0) {
    t_elapse = 300;
  }
  RunInterruptStart(mob, type, end_x, end_y, static_cast<uint32_t>(t_elapse));
}

void ServerMobSystem::RunInterruptGround(ServerMob& mob, uint32_t t_elapse) {
  auto& it = mob.interrupt;
  uint32_t rest = t_elapse;

  while (rest > 0) {
    const uint32_t slice_ms = std::min(rest, kInterruptSliceMs);
    rest -= slice_ms;
    const float dt = static_cast<float>(slice_ms) / 1000.0f;

    if (mob.fh == 0 || !g_fhs_.contains(mob.fh)) {
      it.vx = 0.0f;
      return;
    }
    const GameFoothold& fh = g_fhs_.at(mob.fh);
    const FootholdAxis axis = LoadAxis(fh);
    if (!axis.valid) {
      it.vx = 0.0f;
      return;
    }

    // 世界坐标 -> 沿段一维位置（v95 RelPos::SetFromAbsPos）
    float pos = std::fabs(axis.uvx) > 0.5f
                    ? (mob.pos.x - axis.ox) / axis.uvx
                    : (mob.pos.y - axis.oy) / axis.uvy;

    const float adx = std::fabs(mob.pos.x - it.end_x);
    const float dir_v = it.vx > 0.0f ? 1.0f : (it.vx < 0.0f ? -1.0f : 0.0f);
    // "收到距终点 remaining 处"：世界 x 距离换算成沿段长度
    auto snap = [&](float remaining) {
      if (it.vx != 0.0f && axis.uvx != 0.0f) {
        pos += dir_v * remaining / axis.uvx;
      }
    };
    // 5/7/6/8/9 档共用的停止与减速规则
    auto stop_or_decel = [&]() {
      const float slow_near = (it.type == 6 || it.type == 8 || it.type == 9)
                                  ? kSlowNearFar
                                  : kSlowNear;
      if (adx < kStopNear) {
        it.vx = 0.0f;
      } else if (adx < slow_near) {
        it.vx /= kSlowDiv;
        snap(adx);
      } else {
        // 默认档：按 drag / mass 恒定减速，梯形积分推进
        const float decel = it.drag / kMobMass;
        const float v_old = it.vx;
        it.vx = DecSpeed(it.vx, decel, dt);
        pos += (v_old + it.vx) * 0.5f * dt;
      }
    };

    if (it.type == 3 || it.type == 4) {
      // 3/4 档：还远时按 5/7 档处理，进入 access+40 后减速并收到 access
      if (adx < it.access + kAccessPad) {
        it.vx /= kSlowDiv;
        if (it.vx != 0.0f) {
          snap(std::fabs(adx - it.access));
        }
      } else {
        stop_or_decel();
      }
    } else if (it.type == 10 || it.type == 12 || it.type == 13) {
      // 瞬时位移：只生效一次
      if (!it.relocated && axis.uvx != 0.0f) {
        const float delta = it.end_x / axis.uvx;
        pos += delta;
        it.vx = delta;
        it.relocated = true;
      } else {
        it.vx = 0.0f;
      }
    } else {
      stop_or_decel();
    }

    // 越过段端点：能接上就换段，接不上就减速停在端点
    if (pos < 0.0f) {
      int32_t prev = fh.prev;
      bool wall = prev == 0 || !g_fhs_.contains(prev) ||
                  !g_fhs_.at(prev).k.has_value();
      if (wall) {
        it.vx /= kSlowDiv;
        pos = 0.0f;
      } else {
        const FootholdAxis p_axis = LoadAxis(g_fhs_.at(prev));
        pos += p_axis.len;
        if (pos < 0.0f) {
          pos = 0.0f;
        }
        mob.fh = prev;
      }
    } else if (pos > axis.len) {
      int32_t next = fh.next;
      bool wall = next == 0 || !g_fhs_.contains(next) ||
                  !g_fhs_.at(next).k.has_value();
      if (wall) {
        it.vx /= kSlowDiv;
        pos = axis.len;
      } else {
        const FootholdAxis n_axis = LoadAxis(g_fhs_.at(next));
        pos -= axis.len;
        if (pos > n_axis.len) {
          pos = n_axis.len;
        }
        mob.fh = next;
      }
    }

    const GameFoothold& cur = g_fhs_.at(mob.fh);
    const FootholdAxis cur_axis = LoadAxis(cur);
    if (cur_axis.valid) {
      mob.pos.x = cur_axis.ox + pos * cur_axis.uvx;
      mob.pos.y = cur_axis.oy + pos * cur_axis.uvy;
    }
  }
}

void ServerMobSystem::RunInterruptFlight(ServerMob& mob, uint32_t t_elapse) {
  auto& it = mob.interrupt;
  const auto border = MapInfoGameInstance::LoadMrBorder(map_id_);
  uint32_t rest = t_elapse;

  while (rest > 0) {
    const uint32_t slice_ms = std::min(rest, kInterruptSliceMs);
    rest -= slice_ms;
    const float dt = static_cast<float>(slice_ms) / 1000.0f;
    const float adx = std::fabs(mob.pos.x - it.end_x);
    const float vx_old = it.vx;
    const float vy_old = it.vy;

    switch (it.type) {
      case 3:
      case 4: {
        if (adx < it.access + kAccessPad) {
          it.vx /= kSlowDiv;
        }
        // 竖直方向固定 ±100 贴着终点高度
        it.vy = it.end_y <= mob.pos.y ? -100.0f : 100.0f;
        break;
      }
      case 5:
      case 7: {
        if (adx < it.access + kAccessPad) {
          it.vx /= kSlowDiv;
        }
        break;
      }
      case 6:
      case 8:
      case 9: {
        if (adx < it.access + kSlowNearFar) {
          it.vx /= kSlowDiv;
        }
        break;
      }
      default: {
        const float decel = it.drag / kMobMass;
        it.vx = DecSpeed(it.vx, decel, dt);
        it.vy = DecSpeed(it.vy, decel, dt);
        break;
      }
    }

    mob.pos.x += (vx_old + it.vx) * 0.5f * dt;
    mob.pos.y += (vy_old + it.vy) * 0.5f * dt;
    mob.pos.x = std::clamp(mob.pos.x, border.x + 10.0f, border.w - 10.0f);
    mob.pos.y = std::clamp(mob.pos.y, border.y + 10.0f, border.h - 10.0f);
  }
}

void ServerMobSystem::RunInterruptStep(ServerMob& mob, uint32_t t_elapse) {
  if (mob.fh != 0) {
    RunInterruptGround(mob, t_elapse);
  } else if (mob.move_ability == 4) {
    RunInterruptFlight(mob, t_elapse);
  } else {
    mob.interrupt.vx = 0.0f;
    mob.interrupt.vy = 0.0f;
  }
}

void ServerMobSystem::RunTryJump(ServerMob& mob) {
  if (mob.fh == 0 || mob.move_ability != 3) {
    return;
  }
  mob.vspeed = kJumpSpeed;
  mob.fh = 0;
  mob.action = ServerMobInstance::LoadFineAction(mob.id, u"jump");
  mob.one_shot = false;
}

void ServerMobSystem::RunWalk(ServerMob& mob) {
  SDL_FRect border;
  // rx0/rx1 是地图 life 里给的巡游范围，取绝对值防止 wz 里写反
  border.x = static_cast<float>(std::min(mob.rx0, mob.rx1));
  border.w = static_cast<float>(std::max(mob.rx0, mob.rx1));

  float before_x = mob.pos.x;
  Physic::Walk(mob.pos, static_cast<float>(delta_time_) / 1000.0f, mob.hspeed,
               mob.vspeed, mob.hforce, -mob.hspeed_max, mob.hspeed_max, 0,
               false, mob.fh, border, g_fhs_);

  if (mob.input_x == 0) {
    return;
  }
  if (std::fabs(mob.pos.x - before_x) >= 0.01f) {
    return;
  }

  // 撞墙或走到地形尽头：有跳跃能力的怪起跳，否则掉头
  if (mob.move_ability == 3) {
    RunTryJump(mob);
  } else if (mob.target_id == 0) {
    mob.input_x = static_cast<int8_t>(-mob.input_x);
    mob.flip = mob.input_x > 0;
    mob.think = Window::dt_now_ + 500;
  }
}

bool ServerMobSystem::RunFall(ServerMob& mob) {
  if (mob.fh != 0) {
    return false;
  }
  auto border = MapInfoGameInstance::LoadMrBorder(map_id_);
  border.x = static_cast<float>(mob.rx0);
  border.w = static_cast<float>(mob.rx1);

  // 重力：与人物一致的 2000 px/s^2
  mob.vspeed += static_cast<float>(delta_time_) * 2.0f;

  bool falling =
      Physic::Fall(mob.pos, static_cast<float>(delta_time_) / 1000.0f,
                   mob.hspeed, mob.vspeed, kFallVspeedMin, kFallVspeedMax,
                   border, true, true, mob.fh, mob.page, g_fhs_, INT32_MAX);
  if (!falling) {
    // 落地
    mob.vspeed = 0;
    mob.hspeed = 0;
    RunSetAction(mob, LoadDefaultAction(mob));
    return false;
  }
  return true;
}

void ServerMobSystem::RunFly(ServerMob& mob) {
  const float dt = static_cast<float>(delta_time_) / 1000.0f;
  const float speed = mob.fly_speed;

  auto& clients = ServerClientInstance::clients_;
  if (mob.target_id != 0 && clients.contains(mob.target_id) &&
      clients.at(mob.target_id).player_t.character != nullptr) {
    const auto& state = *clients.at(mob.target_id).player_t.character->state;
    float dx = state.x - mob.pos.x;
    float dy = state.y - mob.pos.y;
    if (std::fabs(dx) >= kFlyDeadZone) {
      mob.pos.x += (dx > 0 ? speed : -speed) * dt;
      mob.flip = dx > 0;
    }
    if (std::fabs(dy) >= kFlyDeadZone) {
      mob.pos.y += (dy > 0 ? speed : -speed) * dt / 2;
    }
  } else if (mob.input_x != 0) {
    mob.pos.x += static_cast<float>(mob.input_x) * speed * dt;
    mob.flip = mob.input_x > 0;
  }

  // 限制在地图范围内
  auto border = MapInfoGameInstance::LoadMrBorder(map_id_);
  mob.pos.x = std::clamp(mob.pos.x, border.x + 10.0f, border.w - 10.0f);
  mob.pos.y = std::clamp(mob.pos.y, border.y + 10.0f, border.h - 10.0f);
}

void ServerMobSystem::RunPhysics(ServerMob& mob) {
  // 被打断位移（击退等）进行中：这段时间由 RunInterruptStep 接管，不走路
  if (mob.interrupt.active) {
    if (Window::dt_now_ >= mob.interrupt.end_time) {
      mob.interrupt.active = false;
      mob.interrupt.vx = 0.0f;
      mob.interrupt.vy = 0.0f;
    } else {
      RunInterruptStep(mob, delta_time_);
      return;
    }
  }

  if (mob.move_ability == 4) {
    RunFly(mob);
    return;
  }
  if (mob.fh != 0 && !g_fhs_.contains(mob.fh)) {
    // 落脚点不在当前地图数据里：让怪落回地面，避免 Physic 里 at() 抛异常
    mob.fh = 0;
    mob.hspeed = 0;
    mob.vspeed = 0;
    return;
  }
  if (mob.fh == 0) {
    RunFall(mob);
    return;
  }
  RunWalk(mob);
}

void ServerMobSystem::RunHitAction(ServerMob& mob) {
  auto delay = ServerMobInstance::LoadActionDelay(mob.id, u"hit1");
  if (delay == 0) {
    delay = 300;
  }
  mob.action = ServerMobInstance::LoadFineAction(mob.id, u"hit1");
  mob.one_shot = true;
  mob.action_end = Window::dt_now_ + delay;
  mob.attack_hit = 0;
}

bool ServerMobSystem::RunHitting(ServerMob& mob) {
  if (mob.hits.empty()) {
    return false;
  }
  const auto current_time = Window::dt_time_;

  // 1) 结算本帧累积伤害
  int64_t total_damage = 0;
  for (auto& b : mob.hits | std::views::values) {
    if (b.hit_num != 0) {
      total_damage += b.hit_num;
      b.hit_num = 0;
    }
  }
  mob.hp -= total_damage;

  const auto& first = mob.hits.begin()->second;
  const auto& last = mob.hits.rbegin()->second;

  // 2) 死亡判定：HP 归零且最后一击已过期
  if (mob.hp <= 0 && last.hit_time < current_time) {
    const auto killer_id = last.hit_id;
    mob.hits.clear();
    RunDie(mob, killer_id);
    return true;
  }

  // 3) 仇恨：打我的就是目标
  mob.hate_id = last.hit_id;
  if (CanAttack(mob)) {
    mob.target_id = last.hit_id;
    mob.chase_end = Window::dt_now_ + kChaseMs;
  }

  if (first.hit_time > current_time) {
    return false;
  }

  if (first.hit_push) {
    // 4) 击退：转向 + nMoveType 0 档的减速滑行 + 受击动作
    mob.flip = !first.left;
    mob.hits.erase(mob.hits.begin());
    RunHitAction(mob);
    // v95：受击位移走 nMoveType 0（初速 130px/s、drag 40000、mass 100 ⇒ 400px/s²）
    const float dir = first.left ? 1.0f : -1.0f;
    RunInterrupt(mob, 0, mob.pos.x + dir * 130.0f, mob.pos.y);
    return true;
  }

  mob.hits.erase(mob.hits.begin());
  return false;
}

void ServerMobSystem::RunDie(ServerMob& mob, uint64_t client_id) {
  mob.hate_id = 0;
  mob.target_id = 0;
  mob.one_shot = false;
  mob.attack_hit = 0;
  mob.interrupt = MobInterrupt{};
  mob.hspeed = 0;
  mob.vspeed = 0;
  mob.hforce = 0;
  mob.input_x = 0;
  mob.action = ServerMobInstance::LoadFineAction(mob.id, u"die1");

  fbs::ServerMobDieT smd;
  smd.client_id = client_id;
  smd.mob_index = mob.index;

  fbs::MobEventUnionUnion muu;
  muu.Set(std::move(smd));
  events_.payload.push_back(std::move(muu));

  std::vector<fbs::DropT> dts =
      ServerMobInstance::CreateMobDts(mob.id, mob.pos.x, mob.pos.y, mob.page);

  fbs::ServerMobDropT smt;
  smt.mob_index = mob.index;
  smt.drop = ServerDropInstance::CreateDts(dts, map_id_);
  for (const auto& dt : smt.drop) {
    ServerDropInstance::SaveDrop(map_id_, *dt);
  }

  fbs::MobEventUnionUnion mu;
  mu.Set(std::move(smt));
  events_.payload.push_back(std::move(mu));

  mob.debuffs.clear();
}

void ServerMobSystem::RunNetworkSyncPos(ServerMob& mob, ServerMob& o_mob) {
  if (o_mob.pos.x != mob.pos.x || o_mob.pos.y != mob.pos.y) {
    fbs::MovementT mv;
    mv.x1 = o_mob.pos.x;
    mv.y1 = o_mob.pos.y;
    mv.x2 = mob.pos.x;
    mv.y2 = mob.pos.y;
    mv.page = mob.page;
    mv.time = std::min(static_cast<uint32_t>(Window::delta_time_),
                       kMinFrameIntervalMs);

    fbs::ServerMobMvT mvt;
    mvt.mob_index = mob.index;
    mvt.payload = std::make_unique<fbs::MovementT>(std::move(mv));

    fbs::MobEventUnionUnion muu;
    muu.Set(std::move(mvt));
    events_.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::RunNetworkSyncFlip(ServerMob& mob, ServerMob& o_mob) {
  if (o_mob.flip != mob.flip) {
    fbs::FlipT ft;
    ft.flip = mob.flip;

    fbs::ServerMobFlipT smf;
    smf.mob_index = mob.index;
    smf.payload = std::make_unique<fbs::FlipT>(std::move(ft));

    fbs::MobEventUnionUnion muu;
    muu.Set(std::move(smf));
    events_.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::RunNetworkSyncAction(ServerMob& mob, ServerMob& o_mob) {
  if (o_mob.action != mob.action) {
    fbs::ActionT a;
    a.action = std::string{mob.action.begin(), mob.action.end()};
    a.action_animate = true;
    a.action_index = 0;

    fbs::ServerMobActionT sma;
    sma.mob_index = mob.index;
    sma.payload = std::make_unique<fbs::ActionT>(std::move(a));

    fbs::MobEventUnionUnion muu;
    muu.Set(std::move(sma));
    events_.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::RunNetworkSyncHp(ServerMob& mob, ServerMob& o_mob) {
  if (o_mob.hp != mob.hp) {
    fbs::StateT st;
    st.state = fbs::StateEnum_HP;
    st.val = mob.hp;

    fbs::ServerMobStateT smb;
    smb.mob_index = mob.index;
    smb.payload.push_back(std::make_unique<fbs::StateT>(std::move(st)));

    fbs::MobEventUnionUnion muu;
    muu.Set(std::move(smb));
    events_.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::RunNetworkSync(ServerMob& mob, ServerMob& o_mob) {
  RunNetworkSyncPos(mob, o_mob);
  RunNetworkSyncFlip(mob, o_mob);
  RunNetworkSyncAction(mob, o_mob);
  RunNetworkSyncHp(mob, o_mob);
}

void ServerMobSystem::RunStateMachine(ServerMob& mob) {
  ServerMob o_mob = mob;

  if (mob.hp > 0) {
    // 1) 受击（可能直接死亡）
    if (!RunHitting(mob)) {
      if (mob.one_shot) {
        // 2) 一次性动作（攻击 / 受击）：推进到判定与结束时刻
        if (mob.attack_hit != 0 && Window::dt_now_ >= mob.attack_hit) {
          RunAttackHit(mob);
        }
        if (Window::dt_now_ >= mob.action_end) {
          RunSetAction(mob, LoadDefaultAction(mob));
          mob.attack_hit = 0;
        }
      } else if (mob.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
        // 3) 眩晕：停止思考，原地待机
        mob.input_x = 0;
        mob.hspeed = 0;
        RunSetAction(mob, LoadDefaultAction(mob));
      } else {
        // 4) 正常思考：选目标 -> 攻击 -> 移动
        RunTarget(mob);
        if (!RunTryAttack(mob)) {
          RunInput(mob);
          RunMoveAction(mob);
        }
      }
    }
  }

  RunPhysics(mob);
  RunNetworkSync(mob, o_mob);
}

void ServerMobSystem::RunSend() {
  if (events_.payload.empty()) {
    return;
  }
  events_.map_id = map_id_;
  auto& clients = ServerSceneInstance::scenes_[map_id_].clients;
  for (auto client_id : clients) {
    ServerResponse::SendToClient(client_id, events_);
  }
  events_.payload.clear();
}

bool ServerMobSystem::Run() {
  static uint64_t last = SDL_GetTicks();

  delta_time_ = static_cast<uint32_t>(Window::dt_now_ - last);
  if (delta_time_ < kMinFrameIntervalMs) {
    return true;
  }
  last = Window::dt_now_;
  delta_time_ = std::min(delta_time_, kMinFrameIntervalMs);

  auto& scenes = ServerSceneInstance::scenes_;
  for (auto& sc : scenes | std::views::values) {
    if (sc.mobs.empty() || sc.clients.empty()) {
      continue;
    }
    map_id_ = sc.map_id;
    g_fhs_.clear();
    for (const auto& [key, value] : sc.fhs) {
      g_fhs_.emplace(key, value.fh);
    }
    for (auto& mob : sc.mobs | std::views::values) {
      RunStateMachine(mob);
    }
    RunSend();
  }
  return true;
}

}  // namespace sdlms
