#include "src/server/server_system/server_mob_system.h"

#include "src/client/game/game_debuff.h"
#include "src/client/game/game_effect.h"
#include "src/client/game_instance/ladderrope_game_instance.h"
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
constexpr float kFlyDeadZone = 10.0f;   // 飞行到点死区
constexpr uint32_t kWanderMinMs = 330;  // 巡游换向间隔
constexpr uint32_t kWanderMaxMs = 990;

// 仇恨 / 攻击
constexpr float kAggroRange = 280.0f;   // 主动仇恨水平距离
constexpr float kAggroHeight = 120.0f;  // 主动仇恨高度差
constexpr float kLoseRange = 900.0f;    // 放弃追击水平距离
constexpr float kLoseHeight = 400.0f;   // 放弃追击高度差
constexpr uint32_t kChaseMs = 180'000;  // 追击持续时间（v95: 180~240s）
constexpr uint32_t kAttackGapMs = 300;  // 动作结束后的攻击间隔

// 区域攻击（v95 IsTargetInAttackRange 的 nType 3/4 分支）
constexpr uint32_t kAreaCooldownMs = 2500;  // m_tLastAreaAttack 硬冷却
constexpr float kAreaFootSearch = 100.0f;   // GetFootholdRange 的竖直搜索半径
constexpr float kBulletAimInset = 10.0f;    // 子弹瞄准点水平/竖直内缩

// 迷惑（Dazzle）：怪打怪
constexpr uint64_t kDazzleBodyGapMs = 2000;   // v95 Update：身体撞击间隔
constexpr uint64_t kHitByMobGapMs = 2000;     // v95 SetDamagedByMob：普通怪被怪打间隔
constexpr uint64_t kHitByMobGapEscortMs = 1500;  // 护送怪的被怪打间隔
constexpr float kDazzleSearchWidth = 300.0f;     // v95 FindNearestMob 搜索框半宽
constexpr float kDazzleSearchHeight = 100.0f;    // 搜索框半高
constexpr float kDazzleRoughGap = 40.0f;         // |dx|/10 + |dy|/3 的粗略近距阈值

// 护送（v95 OnEscortFullPath / CollisionDetectEscortDest）
constexpr float kEscortWindow = 25.0f;  // 穿越判定窗口（±25）
// 爬梯 / 绳（v95 WorkUpdateActiveLadderOrRope）
constexpr float kRopeSnapTolerance = 10.0f;  // GetLadderOrRope 的 x 容差 ±10
constexpr float kRopeClimbScale = 3.0f;      // 爬速 = walkSpeed × inputY × 3.0
constexpr float kRopeProbeUp = -20.0f;       // 上梯探针偏移
constexpr float kRopeProbeDown = 10.0f;      // 下梯探针偏移
constexpr float kRopeNeedHeight = 100.0f;    // 目标高差超过它才考虑爬梯

// 数字 -> u16string（动作名拼接用）
std::u16string NumToU16(int32_t value) {
  std::string narrow = std::to_string(value);
  return std::u16string(narrow.begin(), narrow.end());
}

// 死亡后原地复活：6 秒后开始渐显，1 秒渐显完成
constexpr uint64_t kReviveDelayMs = 6000;
constexpr uint64_t kReviveFadeMs = 1000;

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

float MobSpeedScale(const ServerMob& mob) {
  float scale = 1.0f;
  auto it = mob.debuffs.find(GameDebuff::DebuffType::kThreaten);
  if (it != mob.debuffs.end()) {
    const int32_t down =
        static_cast<int32_t>(std::clamp<int64_t>(it->second.db.val, 0, 100));
    scale *= std::max(0.1f, (100.0f - static_cast<float>(down)) / 100.0f);
  }
  auto slow = mob.debuffs.find(GameDebuff::DebuffType::kSlow);
  if (slow != mob.debuffs.end()) {
    const int32_t down =
        static_cast<int32_t>(std::clamp<int64_t>(slow->second.db.val, 0, 100));
    scale *= std::max(0.1f, (100.0f - static_cast<float>(down)) / 100.0f);
  }
  auto freeze = mob.debuffs.find(GameDebuff::DebuffType::kFreeze);
  if (freeze != mob.debuffs.end()) {
    const int32_t down =
        static_cast<int32_t>(std::clamp<int64_t>(freeze->second.db.val, 0, 100));
    scale *= std::max(0.1f, (100.0f - static_cast<float>(down)) / 100.0f);
  }
  return std::max(0.1f, scale);
}

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

// v95 adjust_rect(rc, x, y, bFlip)：先按朝向镜像，再平移到 (x, y)
SDL_FRect AdjustRect(SDL_FRect rect, float x, float y, bool flip) {
  if (flip) {
    // 镜像：swap(-left, -right)，宽度不变
    rect.x = -(rect.x + rect.w);
  }
  rect.x += x;
  rect.y += y;
  return rect;
}

bool PointInRect(const SDL_FRect& rect, float x, float y) {
  return x >= rect.x && x <= rect.x + rect.w && y >= rect.y &&
         y <= rect.y + rect.h;
}

float Lerp(float a, float b, float t) { return a + (b - a) * t; }

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
    case 6: {  // 护送：路线由服务器下发/维护，没有路线就原地不动
      if (RunEscort(mob)) {
        return;
      }
      RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"stand"));
      mob.hspeed = 0;
      return;
    }
    default: {  // 1 / 3：地面行走
      if (mob.fh == 0) {
        RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"jump"));
        return;
      }
      if (mob.input_x != 0) {
        mob.flip = mob.input_x > 0;
        RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"move"));
        const float speed =
            mob.hspeed_max * MobSpeedScale(mob);
        mob.hspeed = mob.input_x > 0 ? speed : -speed;
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
  // 13/28 技能怪没有 attackN，只要会技能也算"会攻击"
  return !ServerMobInstance::LoadMobAttacks(mob.id).empty() ||
         !ServerMobInstance::LoadMobSkills(mob.id).empty();
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
  // 被迷惑的怪不打玩家：目标是"最近的未被迷惑的怪"（v95 TryFirstAttack 分支 A）
  if (mob.dazzle_owner != 0) {
    mob.target_id = 0;
    RunDazzleTarget(mob);
    return;
  }
  const auto now = Window::dt_now_;
  auto& clients = ServerClientInstance::clients_;

  // 1) 目标失效检查
  if (mob.target_id != 0) {
    bool valid = clients.contains(mob.target_id) &&
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
  // 无目标：每 330~990ms 随机掷一次方向（-1 / 0 / 1），窗口内保持不变
  if (now < mob.think) {
    return;
  }

  std::uniform_int_distribution<int32_t> dist(-1, 1);
  mob.input_x = Sign(static_cast<float>(dist(RandomGameInstance::gen_)));
  std::uniform_int_distribution<uint32_t> dist2(kWanderMinMs, kWanderMaxMs);
  mob.think = now + dist2(RandomGameInstance::gen_);

  auto& clients = ServerClientInstance::clients_;
  if (mob.target_id != 0 && clients.contains(mob.target_id)) {
    const auto& state = *clients.at(mob.target_id).player_t.character->state;
    float dx = state.x - mob.pos.x;
    mob.input_x = dx > 0 ? 1 : -1;
    return;
  }
}

bool ServerMobSystem::RunAttackRange(const ServerMob& mob,
                                     const MobAttack& attack,
                                     const SDL_FRect& target) {
  // flip == true 表示朝右；素材默认朝左
  const float facing = mob.flip ? 1.0f : -1.0f;
  switch (attack.type) {
    case MobAttack::RangeType::kArea: {
      // 区域攻击不再用"整段区域矩形"近似：候选判定见 RunAreaCandidate，
      // 命中判定见 RunAttackTimers 里的逐块矩形
      return false;
    }
    case MobAttack::RangeType::kRect: {
      SDL_FRect rect;
      rect.w = std::fabs(attack.rect.w);
      rect.h = std::fabs(attack.rect.h);
      rect.y =
          mob.pos.y + std::min(attack.rect.y, attack.rect.y + attack.rect.h);
      if (mob.flip) {
        rect.x = mob.pos.x - (attack.rect.x + attack.rect.w);
      } else {
        rect.x = mob.pos.x + attack.rect.x;
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
  // 区域攻击的位掩码按候选下标缓存（v95 TARGETINFO.dwTargetInfo）
  std::vector<uint32_t> area_masks(attacks.size(), 0);
  for (size_t i = 0; i < attacks.size(); ++i) {
    if (attacks[i].inactive) {
      continue;
    }
    // v95 bSpecialAttack：没有蓄力就绪时该攻击不可用
    if (attacks[i].special_attack && !mob.attack_ready) {
      continue;
    }
    switch (attacks[i].ntype) {
      case MobAttack::NType::kAreaGround:
      case MobAttack::NType::kAreaSelf: {
        uint32_t mask = 0;
        if (!RunAreaCandidate(mob, attacks[i], target, &mask)) {
          continue;
        }
        area_masks[i] = mask;
        break;
      }
      default: {
        if (!RunAttackRange(mob, attacks[i], target)) {
          continue;
        }
        break;
      }
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
  // v95 DoAttack：攻击特效层在 t0 + effectAfter 出现（与判定时刻无关）。
  // 多层规则与 CMob::ProcessAttack 一致：先 effect，再依次试 effect0/effect1…，
  // 查到不存在为止。
  {
    const std::u16string prefix =
        mob.id + u".img/" + attack.action + u"/info/";
    for (int32_t layer = -1; layer < 8; ++layer) {
      const std::u16string name =
          layer < 0 ? u"effect" : u"effect" + NumToU16(layer);
      const std::u16string path = prefix + name;
      auto* node = ServerMobInstance::LoadMobNode(mob.id);
      if (node == nullptr || node->find(path) == nullptr) {
        if (layer < 0 && attack.effect_path.empty()) {
          continue;
        }
        break;
      }
      RunSendEffect(mob, 0, path, mob.pos.x, mob.pos.y, !mob.flip,
                attack.effect_after);
    }
  }
  // v95 的攻击条目/子弹是挂在怪物身上、按绝对时刻结算的独立列表，
  // 发起新攻击时不清空（旧条目自然到期），只在死亡/复活时清理
  mob.area_mask = 0;
  mob.attack_index = static_cast<int32_t>(index);

  if (attack.ntype == MobAttack::NType::kBullet) {
    // 远程子弹：判定由 RunUpdateBullets 逐帧做，没有统一的 attack_hit
    mob.attack_hit = 0;
    RunBulletEntries(mob, attack, static_cast<int32_t>(index), target);
  } else if (attack.ntype == MobAttack::NType::kAreaGround ||
             attack.ntype == MobAttack::NType::kAreaSelf) {
    // 区域攻击：v95 DoAttack 在此刷新 m_tLastAreaAttack 并逐块生成条目
    mob.last_area_attack = now;
    mob.attack_hit = 0;
    mob.area_mask = area_masks[index];
    RunAreaEntries(mob, attack, static_cast<int32_t>(index), area_masks[index]);
  } else {
    mob.attack_hit = now + hit_after;
  }
  mob.attack_cd = now + delay + kAttackGapMs;
  // v95 DoAttack：特殊攻击消耗蓄力
  if (attack.special_attack) {
    mob.attack_ready = false;
  }
  mob.input_x = 0;
  mob.hspeed = 0;
  mob.hforce = 0;
  return true;
}

// 把"怪物攻击玩家"事件打包下发：只给怪物攻击力，伤害浮动与减伤由客户端结算。
void ServerMobSystem::RunSendMobDamage(ServerMob& mob, uint64_t client_id,
                                      const MobAttack& attack) {
  int32_t pad = ServerMobInstance::LoadMobInfoInt(mob.id, u"PADamage", 0);
  for (const auto& [type, debuff] : mob.debuffs) {
    if (type == GameDebuff::DebuffType::kDisorder) {
      pad -= static_cast<int32_t>(debuff.db.val);
    }
  }
  pad = std::clamp(pad + attack.pad, 0, 29999);
  if (pad <= 0) {
    pad = 1;
  }

  fbs::ServerMobAttackPlayerT event;
  // v95 SetDamaged：deadlyAttack 把目标打到剩 1 HP，mpBurn 的魔法伤害强制为 1
  if (attack.deadly) {
    event.flags |= 1;
  }
  if (attack.mp_burn) {
    event.flags |= 2;
  }
  if (attack.tremble) {
    event.flags |= 4;
  }
  event.mob_index = mob.index;
  event.client_id = client_id;
  // v95 SetDamaged：deadlyAttack 把目标打到剩 1 HP，mpBurn 的魔法伤害强制为 1
  if (attack.deadly) {
    event.flags |= 1;
  }
  if (attack.mp_burn) {
    event.flags |= 2;
  }
  if (attack.tremble) {
    event.flags |= 4;
  }
  event.payload = std::make_unique<fbs::AttackT>(fbs::AttackT{
      .delay = Window::dt_time_,
      .num = static_cast<uint64_t>(pad),
      .x = mob.pos.x,
      .y = mob.pos.y,
      .type = 0,
  });

  fbs::MobEventUnionUnion muu;
  muu.Set(std::move(event));
  events_.payload.push_back(std::move(muu));

  // v95 SetDamaged 的命中特效 sHit：在受害者身体矩形内随机一点播放
  if (!attack.hit_path.empty()) {
    auto target = LoadTargetRect(client_id);
    float hx = mob.pos.x;
    float hy = mob.pos.y;
    if (target.w > 0.0f) {
      std::uniform_real_distribution<float> rx(target.x, target.x + target.w);
      std::uniform_real_distribution<float> ry(target.y, target.y + target.h);
      hx = rx(RandomGameInstance::gen_);
      hy = ry(RandomGameInstance::gen_);
    }
    RunSendEffect(mob, 0, attack.hit_path, hx, hy, !mob.flip, 0);
  }
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

  RunSendMobDamage(mob, mob.target_id, attack);

  // v95 bRush：冲锋攻击命中后沿目标方向继续推进（nMoveType 14）
  if (attack.rush) {
    const float dir = target.x + target.w / 2.0f > mob.pos.x ? 1.0f : -1.0f;
    RunInterruptDistance(mob, 120.0f, dir > 0.0f);
  }
}

// 自爆在 RunDie 之后不再重复触发；死亡时重置标记由复活流程负责

// v95 GetFootholdRange(x, top, bottom)：在某列竖直区间内找 foothold 的 y
bool ServerMobSystem::LoadFootholdY(float x, float top, float bottom,
                                    float* y) {
  bool found = false;
  float best = 0.0f;
  float best_dy = 0.0f;
  const float mid = (top + bottom) * 0.5f;
  for (const auto& [id, fh] : g_fhs_) {
    (void)id;
    if (!fh.k.has_value() || !fh.intercept.has_value()) {
      continue;  // 没有斜率的是墙面，不是可站立的 foothold
    }
    if (x < std::min(fh.x1, fh.x2) || x > std::max(fh.x1, fh.x2)) {
      continue;
    }
    const float fy = fh.k.value() * x + fh.intercept.value();
    if (fy < std::min(top, bottom) || fy > std::max(top, bottom)) {
      continue;
    }
    const float dy = std::fabs(fy - mid);
    if (!found || dy < best_dy) {
      found = true;
      best = fy;
      best_dy = dy;
    }
  }
  if (found) {
    *y = best;
  }
  return found;
}

bool ServerMobSystem::RunAreaCandidate(const ServerMob& mob,
                                       const MobAttack& attack,
                                       const SDL_FRect& target,
                                       uint32_t* mask) {
  if (attack.area_count <= 0) {
    return false;
  }
  // 1) 2500ms 硬冷却（v95 m_tLastAreaAttack）
  if (mob.last_area_attack != 0 &&
      Window::dt_now_ - mob.last_area_attack < kAreaCooldownMs) {
    return false;
  }

  // 2) 整段区域矩形：x 由 start / start+areaCount 铺开，y 用单块矩形的高度
  const float w = std::fabs(attack.rect.w);  // 单块区域宽度 W = rb.x - lt.x
  const float dir = mob.flip ? 1.0f : -1.0f;
  SDL_FRect total;
  total.h = std::fabs(attack.rect.h);
  total.y = mob.pos.y + std::min(attack.rect.y, attack.rect.y + attack.rect.h);
  const float x0 = mob.pos.x + dir * w * static_cast<float>(attack.start);
  const float x1 =
      mob.pos.x + dir * w * static_cast<float>(attack.start + attack.area_count);
  total.x = std::min(x0, x1);
  total.w = std::fabs(x1 - x0);
  if (!SDL_HasRectIntersectionFloat(&total, &target)) {
    return false;
  }

  // 3) 从 [start, start+areaCount-1] 里不重复随机抽 attackCount 个区域
  //    （v95 get_random_unique_array）
  const int32_t count = std::min(attack.attack_count, attack.area_count);
  std::vector<int32_t> pool(static_cast<size_t>(attack.area_count));
  for (int32_t i = 0; i < attack.area_count; ++i) {
    pool[static_cast<size_t>(i)] = attack.start + i;
  }
  uint32_t flags = 0;
  for (int32_t k = 0; k < count && !pool.empty(); ++k) {
    std::uniform_int_distribution<size_t> pick(0, pool.size() - 1);
    const size_t at = pick(RandomGameInstance::gen_);
    const int32_t idx = pool[at];
    pool.erase(pool.begin() + static_cast<ptrdiff_t>(at));

    // 4) 逐列找地面：type 3 在 my±100 内找 foothold，type 4 直接用自身 y
    const float x = mob.pos.x + dir * (w / 2.0f + w * static_cast<float>(idx));
    float y = 0.0f;
    bool ok = true;
    if (attack.ntype == MobAttack::NType::kAreaGround) {
      ok = LoadFootholdY(x, mob.pos.y + kAreaFootSearch,
                         mob.pos.y - kAreaFootSearch, &y);
    }
    if (!ok) {
      continue;
    }
    const int32_t bit = idx - attack.start;
    if (bit >= 0 && bit < 32) {
      flags |= 1u << bit;
    }
  }
  if (flags == 0) {
    return false;
  }
  *mask = flags;
  return true;
}

void ServerMobSystem::RunAreaEntries(ServerMob& mob, const MobAttack& attack,
                                     int32_t attack_index, uint32_t mask) {
  if (attack.area_count <= 0) {
    return;
  }
  const float w = std::fabs(attack.rect.w);
  const float dir = mob.flip ? 1.0f : -1.0f;
  const bool left = !mob.flip;
  const auto now = Window::dt_now_;
  const int32_t count = std::min(attack.attack_count, attack.area_count);

  // v95 SetRandTimeForAreaAttack：每块区域预填 [0, randDelayAttack) 的随机延迟
  std::vector<uint64_t> delays(static_cast<size_t>(std::max(count, 0)), 0);
  if (attack.rand_delay_attack > 0) {
    std::uniform_int_distribution<int32_t> dist(0, attack.rand_delay_attack - 1);
    for (auto& d : delays) {
      d = static_cast<uint64_t>(dist(RandomGameInstance::gen_));
    }
  }

  int32_t k = 0;
  for (int32_t bit = 0; bit < attack.area_count && k < count; ++bit) {
    if ((mask & (1u << bit)) == 0) {
      continue;
    }
    const int32_t idx = attack.start + bit;
    const float x = mob.pos.x + dir * (w / 2.0f + w * static_cast<float>(idx));
    float y = mob.pos.y;
    if (attack.ntype == MobAttack::NType::kAreaGround) {
      float fy = 0.0f;
      if (LoadFootholdY(x, mob.pos.y + kAreaFootSearch,
                        mob.pos.y - kAreaFootSearch, &fy)) {
        y = fy;
      }
    }
    MobAttackTimer timer;
    // 每块各自延迟：tCur + rand + tAttackAfter
    timer.time = now + delays[static_cast<size_t>(k)] + attack.attack_after;
    timer.attack_index = attack_index;
    timer.block = idx;
    timer.x = x;
    timer.y = y;
    timer.left = left;
    mob.attack_timers.push_back(timer);
    // v95 DoAttack：每块区域在生成时播放 areaWarning 预警特效
    if (!attack.area_warning_path.empty()) {
      // v95 Effect_ApplyStartDelay：预警在攻击开始时播放（先于命中判定）
      RunSendEffect(mob, 0, attack.area_warning_path, x, y, left, 0);
    }
    ++k;
  }
}

void ServerMobSystem::RunAttackTimers(ServerMob& mob) {
  if (mob.attack_timers.empty()) {
    return;
  }
  const auto& attacks = ServerMobInstance::LoadMobAttacks(mob.id);
  const auto now = Window::dt_now_;
  auto& clients = ServerClientInstance::clients_;

  for (auto it = mob.attack_timers.begin(); it != mob.attack_timers.end();) {
    if (now < it->time) {
      ++it;
      continue;
    }
    const MobAttackTimer timer = *it;
    it = mob.attack_timers.erase(it);
    if (timer.attack_index < 0 ||
        static_cast<size_t>(timer.attack_index) >= attacks.size()) {
      continue;
    }
    if (timer.block < 0) {
      continue;
    }
    const auto& attack = attacks[static_cast<size_t>(timer.attack_index)];
    // 单块区域的判定矩形：先按朝向镜像，再平移到该列中心/地面高度
    const SDL_FRect rect =
        AdjustRect(attack.rect, timer.x, timer.y, timer.left);
    // v95 ProcessAttack：区域攻击对范围内所有玩家结算（这里遍历同图玩家）
    for (const auto& [client_id, client] : clients) {
      if (client.map_id != map_id_ || client.player_t.character == nullptr) {
        continue;
      }
      const auto& state = *client.player_t.character->state;
      if (state.action == "dead") {
        continue;
      }
      auto target = LoadTargetRect(client_id);
      if (target.w <= 0) {
        continue;
      }
      if (!SDL_HasRectIntersectionFloat(&rect, &target)) {
        continue;
      }
      RunSendMobDamage(mob, client_id, attack);
    }
  }
}

void ServerMobSystem::RunBulletEntries(ServerMob& mob, const MobAttack& attack,
                                       int32_t attack_index,
                                       const SDL_FRect& target) {
  // v95 DoAttack nType==2：没有 sBall 就不生成子弹
  if (!attack.has_ball) {
    return;
  }
  const bool left = !mob.flip;
  const float facing = mob.flip ? 1.0f : -1.0f;
  // 子弹起点 = 攻击原点
  const float sx = mob.pos.x + facing * std::fabs(attack.sp.x);
  const float sy = mob.pos.y + attack.sp.y;
  // 瞄准点（v95 IsTargetInAttackRange 的 nType 2 分支）
  float aim_y = sy;
  if (aim_y < target.y + kBulletAimInset) {
    aim_y = target.y + kBulletAimInset;
  } else if (aim_y > target.y + target.h - kBulletAimInset) {
    aim_y = target.y + target.h - kBulletAimInset;
  } else {
    aim_y = target.y + target.h / 2.0f;
  }
  const float center_x = target.x + target.w / 2.0f;
  const float aim_x =
      left ? std::min(center_x, target.x + target.w - kBulletAimInset)
           : std::max(center_x, target.x + kBulletAimInset);

  // 飞行时间 = r * 600 / nBulletSpeed（毫秒）
  const float speed =
      attack.bullet_speed > 0 ? static_cast<float>(attack.bullet_speed) : 100.0f;
  const auto flight = static_cast<uint64_t>(
      std::max(1.0f, static_cast<float>(attack.range) * 600.0f / speed));
  const auto now = Window::dt_now_;

  MobBullet bullet;
  bullet.x1 = sx;
  bullet.y1 = sy;
  bullet.x2 = aim_x;
  bullet.y2 = aim_y;
  bullet.start = now + attack.attack_after;
  bullet.end = bullet.start + flight;
  bullet.attack_index = attack_index;
  bullet.left = left;
  mob.bullets.push_back(bullet);

  // v95 MobBullet 的 sBall：从攻击原点飞到瞄准点的飞行体特效
  if (!attack.ball_path.empty()) {
    fbs::ServerMobEffectT effect_msg;
    effect_msg.mob_index = mob.index;
    effect_msg.kind = 0;
    effect_msg.path = std::string{attack.ball_path.begin(), attack.ball_path.end()};
    effect_msg.x = sx;
    effect_msg.y = sy;
    effect_msg.x2 = aim_x;
    effect_msg.y2 = aim_y;
    effect_msg.move_ms = flight;
    effect_msg.flip = left;
    effect_msg.delay = attack.attack_after;
    fbs::MobEventUnionUnion muu;
    muu.Set(std::move(effect_msg));
    events_.payload.push_back(std::move(muu));
  }

  // 多弹：v95 SetMultiBallTarget 会为 bulletNumber-1 个额外目标各生成一颗子弹，
  // 这里取同图内其他存活玩家作为额外目标（目标不足时不补足）。
  if (attack.bullet_number <= 1) {
    return;
  }
  int32_t extra = attack.bullet_number - 1;
  for (const auto& [client_id, client] : ServerClientInstance::clients_) {
    if (extra <= 0) {
      break;
    }
    if (client_id == mob.target_id || client.map_id != map_id_ ||
        client.player_t.character == nullptr) {
      continue;
    }
    const auto& state = *client.player_t.character->state;
    if (state.action == "dead") {
      continue;
    }
    if (std::fabs(state.x - mob.pos.x) > static_cast<float>(attack.range)) {
      continue;
    }
    auto other = LoadTargetRect(client_id);
    if (other.w <= 0) {
      continue;
    }
    MobBullet b = bullet;
    b.x2 = other.x + other.w / 2.0f;
    b.y2 = other.y + other.h / 2.0f;
    mob.bullets.push_back(b);
    --extra;
  }
}

void ServerMobSystem::RunUpdateBullets(ServerMob& mob) {
  if (mob.bullets.empty()) {
    return;
  }
  const auto& attacks = ServerMobInstance::LoadMobAttacks(mob.id);
  const auto now = Window::dt_now_;
  auto& clients = ServerClientInstance::clients_;

  for (auto it = mob.bullets.begin(); it != mob.bullets.end();) {
    const MobBullet bullet = *it;
    if (now < bullet.start) {
      ++it;
      continue;
    }
    const uint64_t span =
        bullet.end > bullet.start ? bullet.end - bullet.start : 1;
    const float t = std::clamp(
        static_cast<float>(now - bullet.start) / static_cast<float>(span), 0.0f,
        1.0f);
    const float px = Lerp(bullet.x1, bullet.x2, t);
    const float py = Lerp(bullet.y1, bullet.y2, t);

    bool hit = false;
    if (bullet.attack_index >= 0 &&
        static_cast<size_t>(bullet.attack_index) < attacks.size()) {
      const auto& attack = attacks[static_cast<size_t>(bullet.attack_index)];
      // v95 MobBullet::ProcessAttack：逐帧"点是否在目标身体矩形内"
      for (const auto& [client_id, client] : clients) {
        if (client.map_id != map_id_ || client.player_t.character == nullptr) {
          continue;
        }
        const auto& state = *client.player_t.character->state;
        if (state.action == "dead") {
          continue;
        }
        auto target = LoadTargetRect(client_id);
        if (target.w <= 0 || !PointInRect(target, px, py)) {
          continue;
        }
        // v95 bJumpAttack：目标不在跳跃状态时该次伤害被丢弃
        if (attack.jump_attack && state.action != "jump") {
          continue;
        }
        RunSendMobDamage(mob, client_id, attack);
        hit = true;
        break;
      }
    }
    if (hit || now >= bullet.end) {
      it = mob.bullets.erase(it);
    } else {
      ++it;
    }
  }
}

// 只下发一次伤害事件（伤害数值已经在服务端算好）
void ServerMobSystem::RunSendDamageValue(ServerMob& mob, uint64_t client_id,
                                        int64_t damage, float x, float y) {
  fbs::ServerMobAttackPlayerT event;
  event.mob_index = mob.index;
  event.client_id = client_id;
  event.payload = std::make_unique<fbs::AttackT>(fbs::AttackT{
      .delay = Window::dt_time_,
      .num = static_cast<uint64_t>(std::max<int64_t>(1, damage)),
      .x = x,
      .y = y,
      .type = 0,
  });

  fbs::MobEventUnionUnion muu;
  muu.Set(std::move(event));
  events_.payload.push_back(std::move(muu));
}

bool ServerMobSystem::RunTrySkill(ServerMob& mob) {
  if (mob.one_shot) {
    return false;
  }
  if (ServerMobInstance::LoadMobInfoInt(mob.id, u"notAttack", 0) != 0) {
    return false;
  }
  // v95 TryDoingSkill 的门禁里没有眩晕判定，但被眩晕的怪不会思考，这里补齐
  if (mob.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return false;
  }
  const auto& skills = ServerMobInstance::LoadMobSkills(mob.id);
  if (skills.empty() || mob.target_id == 0) {
    return false;
  }
  auto& clients = ServerClientInstance::clients_;
  if (!clients.contains(mob.target_id) ||
      clients.at(mob.target_id).player_t.character == nullptr) {
    return false;
  }
  const auto& state = *clients.at(mob.target_id).player_t.character->state;
  if (state.action == "dead") {
    return false;
  }
  mob.flip = state.x > mob.pos.x;

  const auto now = Window::dt_now_;
  const int64_t max_hp = std::max<int64_t>(
      1, ServerMobInstance::LoadMobInfoInt(mob.id, u"maxHP", 1));
  const int64_t hp_percent = mob.hp * 100 / max_hp;

  std::vector<size_t> candidates;
  std::vector<MobSkillLevel> levels(skills.size());
  for (size_t i = 0; i < skills.size(); ++i) {
    const auto& ref = skills[i];
    auto level = ServerMobInstance::LoadMobSkillLevel(ref.skill_id, ref.slv);
    if (!level.valid) {
      continue;
    }
    // 动作缺失（数据里有 10 只怪整条动作节点缺失）直接跳过，避免"播 stand 放技能"
    const std::u16string action =
        u"skill" + NumToU16(static_cast<int32_t>(ref.action));
    if (!ServerMobInstance::HasMobAction(mob.id, action)) {
      continue;
    }
    // v95 服务端语义：HP 阈值、MP、冷却、次数上限、触发概率
    if (level.hp_below > 0 && hp_percent > level.hp_below) {
      continue;
    }
    if (level.mp_con > mob.mp) {
      continue;
    }
    if (auto it = mob.skill_cd.find(ref.skill_id);
        it != mob.skill_cd.end() && now < it->second) {
      continue;
    }
    if (level.limit > 0) {
      auto it = mob.skill_used.find(ref.skill_id);
      if (it != mob.skill_used.end() && it->second >= level.limit) {
        continue;
      }
    }
    if (level.prop > 0) {
      std::uniform_int_distribution<int32_t> roll(1, 100);
      if (roll(RandomGameInstance::gen_) > level.prop) {
        continue;
      }
    }
    levels[i] = level;
    candidates.push_back(i);
  }
  if (candidates.empty()) {
    return false;
  }

  std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
  const size_t index = candidates[dist(RandomGameInstance::gen_)];
  const auto& ref = skills[index];
  const auto& level = levels[index];
  const std::u16string action =
      u"skill" + NumToU16(static_cast<int32_t>(ref.action));

  auto delay = ServerMobInstance::LoadActionDelay(mob.id, action);
  if (delay == 0) {
    delay = 500;
  }
  // 技能没有 attackAfter：按"动作播完才结算"（v95 客户端不做技能判定，
  // 生效时刻由服务端决定，见 notes/mob-skill-spec.md §8.2）
  const uint64_t hit_after = delay;

  mob.action = action;
  mob.one_shot = true;
  mob.action_end = now + delay + kAttackGapMs;
  mob.attack_hit = 0;
  mob.skill_hit = now + hit_after;
  mob.skill_index = static_cast<int32_t>(index);
  mob.skill_id = ref.skill_id;
  mob.skill_slv = ref.slv;
  mob.mp -= level.mp_con;
  mob.attack_cd = now + delay + kAttackGapMs;
  // 冷却：interval（秒×1000）；缺省时退到 afterDelay，再不行给 1 秒
  uint64_t cooldown = level.interval;
  if (cooldown == 0) {
    cooldown = ref.after_delay;
  }
  if (cooldown == 0) {
    cooldown = 1000;
  }
  mob.skill_cd[ref.skill_id] = now + cooldown;
  ++mob.skill_used[ref.skill_id];
  // MobSkill 的施法特效：Skill/MobSkill.img/<id>/level/<slv>/effect
  if (ServerMobInstance::HasMobSkillEffect(ref.skill_id, ref.slv)) {
    RunSendEffect(mob, 1,
              u"MobSkill/" + NumToU16(ref.skill_id) + u".img/level/" +
                  NumToU16(ref.slv) + u"/effect",
              mob.pos.x, mob.pos.y, !mob.flip, ref.effect_after);
  }
  mob.input_x = 0;
  mob.hspeed = 0;
  mob.hforce = 0;
  return true;
}

void ServerMobSystem::RunSkillHit(ServerMob& mob) {
  mob.skill_hit = 0;
  const auto& skills = ServerMobInstance::LoadMobSkills(mob.id);
  if (mob.skill_index < 0 ||
      static_cast<size_t>(mob.skill_index) >= skills.size()) {
    return;
  }
  const auto& ref = skills[static_cast<size_t>(mob.skill_index)];
  const auto level = ServerMobInstance::LoadMobSkillLevel(ref.skill_id, ref.slv);
  if (!level.valid) {
    return;
  }

  // 技能的影响矩形（相对怪物原点，朝左镜像）
  const SDL_FRect area = AdjustRect(level.area, mob.pos.x, mob.pos.y, !mob.flip);
  const bool has_area = level.area.w != 0 || level.area.h != 0;

  const bool magic = level.mp_con > 0 &&
                     ServerMobInstance::LoadMobInfoInt(mob.id, u"MADamage", 0) >
                         ServerMobInstance::LoadMobInfoInt(mob.id,
                                                           u"PADamage", 0);
  int64_t base = ServerMobInstance::LoadMobInfoInt(
      mob.id, magic ? u"MADamage" : u"PADamage", 0);
  // MobSkill.level.x 是技能数值（伤害%）——这里按百分比放大基础攻击力
  if (level.x > 0) {
    base = base * level.x / 100;
  }
  if (base <= 0) {
    base = 1;
  }

  auto& clients = ServerClientInstance::clients_;
  std::vector<uint64_t> hits;
  for (const auto& [client_id, client] : clients) {
    if (client.map_id != map_id_ || client.player_t.character == nullptr) {
      continue;
    }
    const auto& state = *client.player_t.character->state;
    if (state.action == "dead") {
      continue;
    }
    if (has_area) {
      auto target = LoadTargetRect(client_id);
      if (target.w <= 0 || !SDL_HasRectIntersectionFloat(&area, &target)) {
        continue;
      }
    }
    hits.push_back(client_id);
  }
  if (hits.empty()) {
    return;
  }
  // v95：count 限制命中数（-1 = 不限），randomTarget 决定是否随机挑
  if (level.count > 0 && hits.size() > static_cast<size_t>(level.count)) {
    if (level.random_target) {
      std::shuffle(hits.begin(), hits.end(), RandomGameInstance::gen_);
    }
    hits.resize(static_cast<size_t>(level.count));
  }

  for (auto client_id : hits) {
    RunSendDamageValue(mob, client_id, base, mob.pos.x, mob.pos.y);
    // 元素技能给目标挂状态：火=灼烧、冰=冰冻、雷=眩晕、毒=中毒
    // （time 是持续时间，WZ 单位秒×1000，见 MobSkillLevel.duration）
    if (level.duration > 0) {
      GameDebuff::DebuffType type{};
      bool has = true;
      switch (level.elem) {
        case 1:
          type = GameDebuff::DebuffType::kBurn;
          break;
        case 2:
          type = GameDebuff::DebuffType::kFreeze;
          break;
        case 4:
          type = GameDebuff::DebuffType::kDizz;
          break;
        case 8:
          type = GameDebuff::DebuffType::kPoison;
          break;
        default:
          has = false;
          break;
      }
      if (has) {
        const int64_t value = std::clamp<int64_t>(level.x, 1, 100);
        RunSendPlayerDebuff(client_id, type, value, level.duration);
      }
    }
  }

  // 召唤类技能（level 下的 0..N 是召唤模板 id）：延迟到本帧 mob 循环结束后
  // 再创建，避免在遍历 scene.mobs 的过程中插入导致迭代器失效
  if (!level.summon_ids.empty()) {
    PendingSummon summon;
    summon.map_id = map_id_;
    summon.x = mob.pos.x;
    summon.y = mob.pos.y;
    summon.page = mob.page;
    summon.fh = mob.fh;
    summon.mob_ids = level.summon_ids;
    pending_summons_.push_back(std::move(summon));
  }
}

SDL_FRect ServerMobSystem::LoadMobRect(const ServerMob& mob) {
  const float half = ServerMobInstance::LoadMobHalfWidth(mob.id);
  return SDL_FRect{
      .x = mob.pos.x - half,
      .y = mob.pos.y - half * 2.0f,
      .w = half * 2.0f,
      .h = half * 2.0f,
  };
}

int64_t ServerMobSystem::MobVsMobDamage(const ServerMob& attacker,
                                        const ServerMob& victim, bool magic) {
  // v95 CalcDamage::PDamage(const MobStat*, const MobStat*)：
  // dmg = max(1, rand[0.85,1] × atk.PAD × (100 - 目标 PDRate) / 100)
  int32_t atk = std::clamp(
      ServerMobInstance::LoadMobInfoInt(
          attacker.id, magic ? u"MADamage" : u"PADamage", 0),
      0, 29999);
  int32_t reduce = std::clamp(
      ServerMobInstance::LoadMobInfoInt(
          victim.id, magic ? u"MDRate" : u"PDRate", 0),
      0, 100);
  std::uniform_real_distribution<float> dist(0.85f, 1.0f);
  const float scale = dist(RandomGameInstance::gen_);
  int64_t damage = static_cast<int64_t>(static_cast<float>(atk) * scale *
                                       static_cast<float>(100 - reduce) / 100.0f);
  if (damage <= 0 && !magic) {
    damage = 1;  // 只有 PDamage 有 ≥1 下限
  }
  return damage;
}

void ServerMobSystem::RunSendHitMob(ServerMob& mob, const ServerMob& victim,
                                    int64_t damage) {
  fbs::ServerMobHitMobT event;
  event.mob_index = victim.index;
  event.from_index = mob.index;
  event.payload = std::make_unique<fbs::AttackT>(fbs::AttackT{
      .delay = Window::dt_time_,
      .num = static_cast<uint64_t>(std::max<int64_t>(1, damage)),
      .x = victim.pos.x,
      .y = victim.pos.y,
      .type = 0,
  });

  fbs::MobEventUnionUnion muu;
  muu.Set(std::move(event));
  events_.payload.push_back(std::move(muu));
}

void ServerMobSystem::RunSendEffect(ServerMob& mob, uint8_t kind,
                                    const std::u16string& path, float x,
                                    float y, bool flip, uint64_t delay) {
  if (path.empty()) {
    return;
  }
  fbs::ServerMobEffectT effect_msg;
  effect_msg.mob_index = mob.index;
  effect_msg.kind = kind;
  effect_msg.path = std::string{path.begin(), path.end()};
  effect_msg.x = x;
  effect_msg.y = y;
  effect_msg.x2 = x;
  effect_msg.y2 = y;
  effect_msg.move_ms = 0;
  effect_msg.flip = flip;
  effect_msg.delay = delay;

  fbs::MobEventUnionUnion muu;
  muu.Set(std::move(effect_msg));
  events_.payload.push_back(std::move(muu));
}

void ServerMobSystem::RunSetAttackReady(ServerMob& mob, bool ready) {
  mob.attack_ready = ready;
}

// 给玩家挂状态：走既有的角色状态通道（ServerCharacterState + StateEnum_DEBUFF）
void ServerMobSystem::RunSendPlayerDebuff(uint64_t client_id,
                                          GameDebuff::DebuffType type,
                                          int64_t val, uint64_t duration_ms) {
  auto state = std::make_unique<fbs::StateT>();
  state->state = fbs::StateEnum_DEBUFF;
  state->val = static_cast<int64_t>(type);
  state->sub_val = val;
  state->time = Window::dt_time_ + duration_ms;
  state->delay = Window::dt_time_;

  fbs::ServerCharacterStateT t;
  t.client_id = client_id;
  t.payload.push_back(std::move(state));

  auto it = ServerSceneInstance::scenes_.find(map_id_);
  if (it == ServerSceneInstance::scenes_.end()) {
    return;
  }
  for (auto c : it->second.clients) {
    ServerResponse::SendToClient(c, t);
  }
}

// v95 TryFirstSelfDestruction：nType 0 的攻击框与玩家身体矩形相交即自爆
void ServerMobSystem::RunTrySelfDestruction(ServerMob& mob) {
  if (mob.self_destroyed || mob.dead || mob.hp <= 0) {
    return;
  }
  if (ServerMobInstance::LoadMobInfoInt(mob.id, u"selfDestruction", 0) == 0) {
    return;
  }
  const auto& attacks = ServerMobInstance::LoadMobAttacks(mob.id);
  if (attacks.empty()) {
    return;
  }
  auto& clients = ServerClientInstance::clients_;
  const bool left = !mob.flip;
  std::vector<uint64_t> victims;
  const MobAttack* used = nullptr;
  for (const auto& attack : attacks) {
    if (attack.ntype != MobAttack::NType::kMelee) {
      continue;
    }
    const SDL_FRect rect =
        AdjustRect(attack.rect, mob.pos.x, mob.pos.y, left);
    for (const auto& [client_id, client] : clients) {
      if (client.map_id != map_id_ || client.player_t.character == nullptr) {
        continue;
      }
      const auto& state = *client.player_t.character->state;
      if (state.action == "dead") {
        continue;
      }
      auto target = LoadTargetRect(client_id);
      if (target.w <= 0 || !SDL_HasRectIntersectionFloat(&rect, &target)) {
        continue;
      }
      if (std::find(victims.begin(), victims.end(), client_id) ==
          victims.end()) {
        victims.push_back(client_id);
      }
    }
    if (!victims.empty() && used == nullptr) {
      used = &attack;
    }
  }
  if (victims.empty() || used == nullptr) {
    return;
  }
  mob.self_destroyed = true;
  for (auto client_id : victims) {
    RunSendMobDamage(mob, client_id, *used);
  }
  // 自爆：直接死亡（不做埋点掉落之外的额外处理，RunDie 里有掉落与事件）
  RunDie(mob, 0);
}

void ServerMobSystem::RunDazzleBodyAttack(ServerMob& mob) {
  if (mob.dazzle_owner == 0 || mob.one_shot) {
    return;
  }
  const auto now = Window::dt_now_;
  if (now < mob.dazzle_cd) {
    return;
  }
  if (mobs_ == nullptr) {
    return;
  }
  const SDL_FRect self = LoadMobRect(mob);
  for (auto&& [index, other] : *mobs_) {
    if (index == mob.index || other.hp <= 0 || other.dead) {
      continue;
    }
    // v95 FindBodyAttackMob：目标必须"未被迷惑"且模板 info/bodyAttack 为真
    if (other.dazzle_owner != 0) {
      continue;
    }
    if (ServerMobInstance::LoadMobInfoInt(other.id, u"bodyAttack", 0) == 0) {
      continue;
    }
    const SDL_FRect victim = LoadMobRect(other);
    if (!SDL_HasRectIntersectionFloat(&self, &victim)) {
      continue;
    }
    if (now < other.hit_by_mob) {
      continue;  // 受方 2000ms 节流
    }
    const int64_t damage = MobVsMobDamage(mob, other, false);
    other.hp -= damage;
    other.hit_by_mob = now + kHitByMobGapMs;
    mob.dazzle_cd = now + kDazzleBodyGapMs;
    RunSendHitMob(mob, other, damage);
    // 被撞的怪反手追攻击者（v95 SetDamagedByMob 里的 ChaseTarget）
    if (other.target_id == 0 && other.target_mob == 0) {
      other.target_mob = mob.index;
    }
    break;  // v95 一次只撞一只
  }
}

void ServerMobSystem::RunDazzleTarget(ServerMob& mob) {
  if (mobs_ == nullptr) {
    return;
  }
  // 当前目标失效检查
  if (mob.target_mob != 0) {
    auto it = mobs_->find(mob.target_mob);
    if (it != mobs_->end() && it->second.hp > 0 && !it->second.dead) {
      return;
    }
    mob.target_mob = 0;
  }
  // v95 FindNearestMob：搜索框半宽 300 / 半高 100，选最近的怪
  uint32_t best = 0;
  float best_dist = std::numeric_limits<float>::max();
  for (const auto& [index, other] : *mobs_) {
    if (index == mob.index || other.hp <= 0 || other.dead) {
      continue;
    }
    if (other.dazzle_owner != 0) {
      continue;  // 只打未被迷惑的怪
    }
    if (ServerMobInstance::LoadMobInfoInt(other.id, u"bodyAttack", 0) == 0) {
      continue;
    }
    const float dx = std::fabs(other.pos.x - mob.pos.x);
    const float dy = std::fabs(other.pos.y - mob.pos.y);
    if (dx > kDazzleSearchWidth || dy > kDazzleSearchHeight) {
      continue;
    }
    if (dx < best_dist) {
      best_dist = dx;
      best = index;
    }
  }
  mob.target_mob = best;
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
  it.fixed_distance = false;

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

void ServerMobSystem::RunInterruptDistance(ServerMob& mob, float distance,
                                           bool left, float speed_scale) {
  if (distance <= 0.0f) {
    return;
  }
  auto t_elapse = ServerMobInstance::LoadActionDelay(mob.id, mob.action);
  if (t_elapse == 0) {
    t_elapse = 300;
  }
  const float dir = left ? 1.0f : -1.0f;
  RunInterruptStart(mob, 2, mob.pos.x + dir * distance, mob.pos.y,
                    static_cast<uint32_t>(t_elapse));
  // 095 Power Knockback 的固定击退距离：按动作时长均速移动。
  mob.interrupt.vx = dir * distance / (static_cast<float>(t_elapse) / 1000.0f) *
                     std::max(0.1f, speed_scale);
  mob.interrupt.drag = 0.0f;
  mob.interrupt.fixed_distance = true;
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
    float pos = std::fabs(axis.uvx) > 0.5f ? (mob.pos.x - axis.ox) / axis.uvx
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

    // 固定距离击退：匀速走完，接近终点时不能 snap，否则会产生瞬移。
    auto fixed_step = [&]() {
      const float uv = std::fabs(axis.uvx) > 0.5f ? axis.uvx : axis.uvy;
      if (std::fabs(uv) > 1e-4f) {
        pos += it.vx * dt / uv;
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
    } else if (it.fixed_distance) {
      fixed_step();
    } else {
      stop_or_decel();
    }

    // 越过段端点：能接上就换段，接不上就减速停在端点
    if (pos < 0.0f) {
      int32_t prev = fh.prev;
      bool wall =
          prev == 0 || !g_fhs_.contains(prev) || !g_fhs_.at(prev).k.has_value();
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
      bool wall =
          next == 0 || !g_fhs_.contains(next) || !g_fhs_.at(next).k.has_value();
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
    mob.pos.x = std::clamp(mob.pos.x, (float)mob.rx0, (float)mob.rx1);
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
  // 击退落点不能越过地图边缘，否则怪物会被推出地图。
  const auto border = MapInfoGameInstance::LoadMrBorder(map_id_);
  mob.pos.x = std::clamp(mob.pos.x, border.x + 10.0f, border.w - 10.0f);
  mob.pos.y = std::clamp(mob.pos.y, border.y + 10.0f, border.h - 10.0f);
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
  } else if (mob.target_id != 0) {
    // 追击时被墙挡住：先停下，下一帧继续按玩家位置重新选方向，
    // 不能保持原方向一直顶墙。
    mob.input_x = 0;
    mob.hspeed = 0;
  } else {
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
  border.x = static_cast<float>(std::min(mob.rx0, mob.rx1));
  border.w = static_cast<float>(std::max(mob.rx0, mob.rx1));

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
    // 死亡动作不能因为落地切回 stand，否则尸体会停在死亡帧
    if (!mob.dead) {
      RunSetAction(mob, LoadDefaultAction(mob));
    }
    return false;
  }
  return true;
}

void ServerMobSystem::RunFly(ServerMob& mob) {
  const float dt = static_cast<float>(delta_time_) / 1000.0f;
  const float speed = mob.fly_speed * MobSpeedScale(mob);

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
  // 死亡动画期间停止物理：只播放 die1，不再让尸体下落。
  // 注意这只排除 dead；HP<=0 但攻击 delay 未到时仍按空中 fall 处理。
  if (mob.dead) {
    return;
  }

  // 爬梯 / 绳：吸附时 fh 被清 0，必须在 fall 之前拦截（v95 用 m_pLadderOrRope 顶点）
  if (mob.on_rope) {
    RunRope(mob);
    return;
  }

  // 095/main：只要在地上怪的 fh == 0（跳跃/被打飞），就必须执行 fall，
  // 不能被受击 interrupt 提前 return 挡住。
  if (mob.fh == 0 && mob.move_ability != 4) {
    mob.interrupt.active = false;
    mob.interrupt.vx = 0.0f;
    mob.interrupt.vy = 0.0f;
    RunFall(mob);
    return;
  }

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
  // 地面怪：目标在高处且附近有梯/绳时吸附上去
  if (RunTryRope(mob)) {
    return;
  }
  RunWalk(mob);
}

void ServerMobSystem::RunHitAction(ServerMob& mob) {
  mob.action = ServerMobInstance::LoadFineAction(mob.id, u"hit1");
  // 受击动作由 RunHitCheck 按 hit1 动画总 delay 结束，不走 one_shot 计时
  mob.one_shot = false;
  mob.action_end = 0;
  mob.attack_hit = 0;
}

bool ServerMobSystem::RunHitCheck(ServerMob& mob) {
  // 调用方仅在 mob.hits 非空时进入；这里直接处理队首。
  const auto it = mob.hits.begin();
  const uint64_t hit_time = it->first;

  auto delay = ServerMobInstance::LoadActionDelay(mob.id, u"hit1");
  if (delay == 0) {
    delay = 300;
  }
  if (hit_time + delay > Window::dt_time_) {
    return false;
  }
  mob.hits.erase(it);
  return true;
}

void ServerMobSystem::RunDot(ServerMob& mob) {
  const auto now = Window::dt_time_;
  for (auto it = mob.debuffs.begin(); it != mob.debuffs.end(); ++it) {
    const auto type = it->first;
    auto& sd = it->second;
    if (type != GameDebuff::DebuffType::kBurn &&
        type != GameDebuff::DebuffType::kPoison) {
      continue;
    }
    auto& db = sd.db;
    const int64_t damage = std::max<int64_t>(0, db.val);
    const int interval =
        std::clamp<int>(static_cast<int>(db.val2), 1, 0xFFFF);
    if (damage <= 0 || now >= db.destroy || now < db.delay) {
      continue;
    }
    // DOT 伤害由客户端按“基础魔法攻击 + WZ dot”计算后传入数据库结算。
    // 只扣 HP，不进入 hit 队列，因此不会触发受击动作或击退。
    mob.hp -= damage;

    // DoT 伤害数字：广播给同图所有客户端（含 host），由客户端
    // HandleSAttack 统一画红字。原来直接写本地 EffectGameInstance::data_[7]，
    // 只有跑服务端 tick 的那台机器能看到。
    fbs::ServerMobAttackT dot_t;
    dot_t.client_id = 0;
    dot_t.payload = std::make_unique<fbs::AttackT>(fbs::AttackT{
        .delay = Window::dt_time_,
        .num = static_cast<uint64_t>(damage),
        .x = mob.pos.x,
        .y = mob.pos.y,
        .type = static_cast<uint8_t>(DamageData::DamageColor::kRed),
    });
    if (auto scene = ServerSceneInstance::scenes_.find(map_id_);
        scene != ServerSceneInstance::scenes_.end()) {
      for (auto c : scene->second.clients) {
        ServerResponse::SendToClient(c, dot_t);
      }
    }

    db.delay = now + static_cast<uint64_t>(interval);
  }
}

bool ServerMobSystem::RunHitting(ServerMob& mob) {
  if (mob.hits.empty()) {
    return false;
  }
  const auto current_time = Window::dt_time_;

  // 1) 在 RunHitting 内结算本帧累积伤害（main 分支语义，int64_t 防溢出）。
  int64_t total_damage = 0;
  for (auto& b : mob.hits | std::views::values) {
    if (b.hit_num != 0) {
      total_damage += b.hit_num;
      b.hit_num = 0;
    }
  }
  mob.hp -= total_damage;

  // 2) 095 的 hit 状态同时是受击冷却：这期间到达的后续攻击只结算伤害，
  // 不能再触发第二次 hit 动作或击退。
  if (mob.action.starts_with(u"hit")) {
    // 特例：最新一次攻击已经致死，且它的攻击 delay 已到，
    // 不再等当前 hit1 播完，强制转入死亡。
    const auto& latest = mob.hits.rbegin()->second;
    if (mob.hp <= 0 && latest.hit_time <= current_time) {
      const auto killer_id = latest.hit_id;
      mob.hits.clear();
      RunDie(mob, killer_id);
      return true;
    }
    for (auto& b : mob.hits | std::views::values) {
      if (b.hit_time <= current_time) {
        b.hit_reaction = false;
        b.hit_push = false;
        b.knockback = 0.0f;
      }
    }
    return true;
  }

  // 清除已经错过 hit 反应窗口、只结算过伤害的队列项。
  while (!mob.hits.empty()) {
    const auto it = mob.hits.begin();
    if (it->second.hit_reaction) {
      break;
    }
    mob.hits.erase(it);
  }
  if (mob.hits.empty()) {
    return false;
  }

  // 3) 取首/尾；hits 是有序容器，rbegin 是"最后一次命中"。
  const auto& first = mob.hits.begin()->second;
  const auto& last = mob.hits.rbegin()->second;

  // 首击的 hit_time 未到，暂不推进动作（伤害已结算）。
  if (first.hit_time > current_time) {
    return false;
  }

  // 4) 死亡判定：HP 归零且最后一击已过期。
  if (mob.hp <= 0 && last.hit_time < current_time) {
    const auto killer_id = last.hit_id;  // clear 前保存
    mob.hits.clear();
    RunDie(mob, killer_id);
    return true;
  }

  // 5) 受击目标就是攻击者：之后 AI 统一追击 target_id。
  mob.target_id = last.hit_id;
  mob.chase_end = Window::dt_now_ + kChaseMs;

  // 6) 095：命中进入 hit 状态；hit_push 只决定是否击退。
  mob.flip = !first.left;
  mob.hspeed = 0.0f;
  mob.hforce = 0.0f;
  RunHitAction(mob);
  if (first.knockback > 0.0f && (mob.fh != 0 || mob.move_ability == 4)) {
    RunInterruptDistance(mob, first.knockback, first.left,
                         first.knockback_speed);
  } else if (first.hit_push && (mob.fh != 0 || mob.move_ability == 4)) {
    // 095 的击退距离由 RunInterrupt 的 nMoveType 0 档负责，不用 hforce。
    const float dir = first.left ? 1.0f : -1.0f;
    RunInterrupt(mob, 0, mob.pos.x + dir * 130.0f, mob.pos.y);
  }
  return true;
}

void ServerMobSystem::RunDie(ServerMob& mob, uint64_t client_id) {
  mob.dead = true;
  mob.reviving = false;
  // 死亡 6 秒后回到 WZ 初始出生点复活。
  mob.revive = Window::dt_now_ + kReviveDelayMs;
  mob.target_id = 0;
  mob.one_shot = false;
  mob.attack_hit = 0;
  mob.skill_hit = 0;
  mob.attack_timers.clear();
  mob.bullets.clear();
  mob.target_mob = 0;
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
  ServerMobInstance::ClearPendingDebuffs(map_id_, mob.index);
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

  // 死亡：6 秒后回到出生点满血复活，并保持 1 秒的渐显阶段。
  if (mob.dead) {
    if (Window::dt_now_ >= mob.revive) {
      mob.dead = false;
      mob.reviving = true;
      mob.revive = Window::dt_now_ + kReviveFadeMs;
      mob.pos = mob.spawn_pos;
      mob.fh = mob.spawn_fh;
      mob.page = mob.spawn_page;
      mob.hp = ServerMobInstance::LoadMobInfoInt(mob.id, u"maxHP", 1);
      mob.mp = ServerMobInstance::LoadMobInfoInt(mob.id, u"maxMP", 100);
      mob.action = u"";
      mob.hits.clear();
      mob.target_id = 0;
      mob.one_shot = false;
      mob.action_end = 0;
      mob.attack_hit = 0;
      mob.skill_hit = 0;
      mob.attack_timers.clear();
      mob.bullets.clear();
      mob.skill_cd.clear();
      mob.skill_used.clear();
      mob.self_destroyed = false;
      mob.dazzle_owner = 0;
      mob.target_mob = 0;
      mob.debuffs.clear();
      RunNetworkSync(mob, o_mob);
    }
    return;
  }
  if (mob.reviving) {
    if (Window::dt_now_ >= mob.revive) {
      mob.reviving = false;
      mob.revive = 0;
      mob.action = LoadDefaultAction(mob);
      RunNetworkSync(mob, o_mob);
    }
    // 渐显结束前：不产生 AI 碰撞/攻击，也不接受玩家攻击。
    return;
  }

  std::erase_if(mob.debuffs, [](const auto& pair) {
    return pair.second.db.destroy <= Window::dt_time_;
  });

  RunDot(mob);

  // 迷惑：身体撞击（怪打怪）与到期清理
  if (mob.dazzle_owner != 0 && Window::dt_now_ >= mob.dazzle_end) {
    mob.dazzle_owner = 0;
    mob.target_mob = 0;
  }
  RunDazzleBodyAttack(mob);

  // v95 ProcessAttack 每帧都会处理"已生成的攻击条目 + 在飞子弹"，
  // 与当前是否在播一次性动作无关（区域攻击的条目会活过动作结束）
  RunAttackTimers(mob);
  RunUpdateBullets(mob);

  if (!mob.dead) {
    bool hit_state = mob.action.starts_with(u"hit");
    // 受击队列必须继续处理，即使 HP 已经 <= 0；死亡要等攻击 delay 到达。
    const bool hit_action = RunHitting(mob);

    // main 的 hit 状态：hit1 动画总 delay 结束后才清 hit，回到 stand。
    // 最后一击 waiting death 期间也允许清掉旧 hit，随后可以播放非 hit 动作。
    if (!mob.dead && hit_state && (mob.hits.empty() || RunHitCheck(mob))) {
      RunSetAction(mob, LoadDefaultAction(mob));
      hit_state = false;
    }

    if (!mob.dead) {
      if (mob.hp <= 0 && mob.hits.empty()) {
        // delay 已过但仍未死亡（例如同时间戳命中/空队列边界），补死亡
        RunDie(mob, mob.target_id);
      } else if (!hit_state && !hit_action) {
        if (mob.one_shot) {
          // 一次性动作（攻击 / 受击）：推进到判定与结束时刻
          if (mob.attack_hit != 0 && Window::dt_now_ >= mob.attack_hit) {
            RunAttackHit(mob);
          }
          if (mob.skill_hit != 0 && Window::dt_now_ >= mob.skill_hit) {
            RunSkillHit(mob);
          }
          if (Window::dt_now_ >= mob.action_end) {
            RunSetAction(mob, LoadDefaultAction(mob));
            mob.attack_hit = 0;
          }
        } else if (mob.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
          // 眩晕：停止思考，原地待机
          mob.input_x = 0;
          mob.hspeed = 0;
          RunSetAction(mob, LoadDefaultAction(mob));
        } else {
          // 正常思考 / 最后一击等待死亡期间：追击 target_id 的玩家
          RunTarget(mob);
          RunTrySelfDestruction(mob);
          // v95：TryDoingSkill 优先于普通攻击
          if (!mob.dead && !RunTrySkill(mob) && !RunTryAttack(mob)) {
            RunInput(mob);
            RunMoveAction(mob);
          }
        }
      }
    }
  }

  // 空血待死亡期间继续物理；dead 后 RunPhysics 会停止死亡动画的下落
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
    mobs_ = &sc.mobs;
    for (auto& mob : sc.mobs | std::views::values) {
      RunStateMachine(mob);
    }
    mobs_ = nullptr;
    RunSend();
  }
  RunPendingSummons();
  ServerMobInstance::RunPendingDebuffs();
  return true;
}

// 技能召唤：把延迟的召唤请求批量创建（复用客户端创建怪物通道，同图广播）
void ServerMobSystem::RunPendingSummons() {
  if (pending_summons_.empty()) {
    return;
  }
  auto summons = std::move(pending_summons_);
  pending_summons_.clear();
  for (const auto& summon : summons) {
    fbs::ClientCreateMobT r;
    r.map_id = summon.map_id;
    for (size_t i = 0; i < summon.mob_ids.size(); ++i) {
      fbs::MobT m;
      m.mob_id = summon.mob_ids[i];
      m.state = std::make_unique<fbs::LifeStateT>(fbs::LifeStateT{
          .x = summon.x + static_cast<float>(i) * 30.0f - 15.0f,
          .y = summon.y,
          .action = "",
          .page = summon.page,
          .fh = static_cast<int16_t>(summon.fh),
      });
      r.mobs.push_back(std::make_unique<fbs::MobT>(std::move(m)));
    }
    ServerMobInstance::HandleCreateMob(r);
  }
}

// 护送：按航点表推进（v95 ±25 窗口 + 越侧穿越判定，到点后按 nStopDuration 停留）
bool ServerMobSystem::RunEscort(ServerMob& mob) {
  if (mob.escort_path.empty()) {
    return false;
  }
  const auto now = Window::dt_now_;
  if (mob.escort_stop != 0) {
    if (now < mob.escort_stop) {
      mob.input_x = 0;
      mob.hspeed = 0;
      RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"stand"));
      return true;
    }
    mob.escort_stop = 0;
  }
  if (mob.escort_index >= mob.escort_path.size()) {
    // v95：走到终点后 RemoveAll，没有路线就不巡游
    mob.escort_path.clear();
    mob.escort_index = 0;
    return false;
  }
  const auto& node = mob.escort_path[mob.escort_index];
  const float dx = node.x - mob.pos.x;
  mob.input_x = dx > 0.0f ? 1 : (dx < 0.0f ? -1 : 0);
  if (mob.input_x != 0) {
    mob.flip = mob.input_x > 0;
  }
  mob.input_y = 0;
  if (std::fabs(dx) <= kEscortWindow) {
    // 到点：推进航点并在需要时进入停留
    ++mob.escort_index;
    if (node.stop_ms > 0) {
      mob.escort_stop = now + node.stop_ms;
    }
    mob.input_x = 0;
  }
  RunSetAction(mob, ServerMobInstance::LoadFineAction(
                        mob.id, mob.input_x != 0 ? u"move" : u"stand"));
  if (mob.input_x != 0) {
    const float speed = mob.hspeed_max * MobSpeedScale(mob);
    mob.hspeed = mob.input_x > 0 ? speed : -speed;
  } else {
    mob.hspeed = 0;
  }
  return true;
}

// 爬梯/绳：目标在高处、附近 ±10 内有梯绳且高度覆盖目标时吸附
bool ServerMobSystem::RunTryRope(ServerMob& mob) {
  if (mob.on_rope || mob.move_ability == 0 || mob.move_ability == 4) {
    return false;
  }
  if (mob.target_id == 0 || mob.fh == 0) {
    return false;
  }
  auto& clients = ServerClientInstance::clients_;
  if (!clients.contains(mob.target_id) ||
      clients.at(mob.target_id).player_t.character == nullptr) {
    return false;
  }
  const auto& state = *clients.at(mob.target_id).player_t.character->state;
  const float dy = state.y - mob.pos.y;
  if (std::fabs(dy) < kRopeNeedHeight) {
    return false;
  }
  // v95 GetLadderOrRope：x 容差 ±10，取最近的梯绳
  const GameLadderrope* best = nullptr;
  int32_t best_id = 0;
  float best_dx = kRopeSnapTolerance;
  for (const auto& [id, lr] : LadderropeGameInstance::data_) {
    const float dx = std::fabs(static_cast<float>(lr.x) - mob.pos.x);
    if (dx > best_dx) {
      continue;
    }
    // 梯绳必须朝目标方向延伸：目标在上方要用 y1 探针，下方用 y2 探针
    const float probe = dy < 0.0f ? lr.y1 + kRopeProbeUp : lr.y2 + kRopeProbeDown;
    if (dy < 0.0f ? probe < state.y : probe > state.y) {
      continue;
    }
    best = &lr;
    best_id = id;
    best_dx = dx;
  }
  if (best == nullptr) {
    return false;
  }
  mob.on_rope = true;
  mob.rope_id = best_id;
  mob.rope_x = static_cast<float>(best->x);
  mob.pos.x = mob.rope_x;
  mob.fh = 0;
  mob.hspeed = 0;
  mob.vspeed = 0;
  mob.interrupt.active = false;
  RunSetAction(mob, ServerMobInstance::LoadFineAction(mob.id, u"rope"));
  return true;
}

// 爬梯推进：每帧按 walkSpeed × inputY × 3（v95 不乘 dt），到端点脱钩
void ServerMobSystem::RunRope(ServerMob& mob) {
  auto it = LadderropeGameInstance::data_.find(mob.rope_id);
  if (it == LadderropeGameInstance::data_.end()) {
    mob.on_rope = false;
    return;
  }
  const auto& lr = it->second;
  const float top = static_cast<float>(std::min(lr.y1, lr.y2));
  const float bottom = static_cast<float>(std::max(lr.y1, lr.y2));
  mob.pos.x = mob.rope_x;

  float dy = 0.0f;
  auto& clients = ServerClientInstance::clients_;
  if (mob.target_id != 0 && clients.contains(mob.target_id) &&
      clients.at(mob.target_id).player_t.character != nullptr) {
    dy = clients.at(mob.target_id).player_t.character->state->y - mob.pos.y;
  }
  int8_t input_y = 0;
  if (std::fabs(dy) > 10.0f) {
    input_y = dy < 0.0f ? -1 : 1;
  }
  // v95：爬速每帧乘 3，不乘 dt
  mob.pos.y += static_cast<float>(input_y) * mob.hspeed_max * kRopeClimbScale;

  bool detach = false;
  if (mob.pos.y >= bottom) {
    mob.pos.y = bottom + 1.0f;  // 到底 → 脱钩站到地面
    detach = true;
  } else if (mob.pos.y <= top) {
    mob.pos.y = top;
    if (input_y < 0) {
      detach = true;  // 到顶且没有上层地板 → 脱钩（落在梯顶平台上由 RunFall 处理）
    }
  }
  RunSetAction(mob, ServerMobInstance::LoadFineAction(
                        mob.id, std::fabs(dy) > 10.0f ? u"rope" : u"rope"));
  if (detach) {
    mob.on_rope = false;
    mob.fh = 0;  // 交给 RunFall 重新落地
    mob.vspeed = 0;
  }
}

// 给护送怪设置一条往返路线（本工程由服务端生成，v95 由服务器 0x130 下发）
void ServerMobSystem::RunSetEscortPath(ServerMob& mob, float x1, float x2,
                                       float y, uint64_t stop_ms) {
  mob.escort_path.clear();
  mob.escort_index = 0;
  mob.escort_stop = 0;
  if (x1 == x2) {
    return;
  }
  // 顺序：先近端再远端，形成往返
  const float near_x = std::fabs(mob.pos.x - x1) <= std::fabs(mob.pos.x - x2)
                           ? x1
                           : x2;
  const float far_x = near_x == x1 ? x2 : x1;
  mob.escort_path.push_back(MobEscortNode{near_x, y, stop_ms});
  mob.escort_path.push_back(MobEscortNode{far_x, y, stop_ms});
}

}  // namespace sdlms
