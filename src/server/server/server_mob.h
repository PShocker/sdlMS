#pragma once

#include "src/common/flatbuffers/common.h"
#include "src/server/server/server_debuff.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

namespace sdlms {

struct MobHit {
  uint64_t hit_id;  // client_id
  uint64_t hit_time;
  bool hit_push;
  int64_t hit_num;
  bool left;
};

// 对应 v95 CVecCtrlMob 的"被打断位移"状态（UpdateActiveInterrupted）
// nMoveType 决定初速度、阻力与停止档位，见 ServerMobSystem::RunInterruptStart
struct MobInterrupt {
  bool active = false;
  int32_t type = 0;    // nMoveType
  float vx = 0;        // 当前水平速度（px/s，已带方向）
  float vy = 0;        // 飞行时的竖直速度（px/s）
  float drag = 0;      // 阻力（减速度 = drag / kMobMass）
  float end_x = 0;     // nMoveEndingX
  float end_y = 0;     // nMoveEndingY
  float access = 0;    // nAccessLimitRange
  uint64_t end_time = 0;
  bool relocated = false;  // nMoveType 10 的瞬时位移只生效一次
};

// Mob.wz/<id>.img/attackN/info 的解析结果，对应 v95 的 MobAttackInfo
struct MobAttack {
  // 攻击范围形状，对应 v95 LoadAttackInfo 按 info/type 展开的三种 range
  enum class RangeType {
    kRect,       // info/range/{lt,rb}        矩形判定
    kTrapezoid,  // info/range/{sp,r}          梯形判定（纵向半高 = 水平距离 / 4）
    kArea,       // info/range/{start,areaCount,attackCount,lt,rb} 区域连击
  };

  std::u16string action;  // 动作名 attackN

  RangeType type = RangeType::kRect;
  bool magic = false;
  bool do_first = false;
  bool inactive = false;

  int32_t pad = 0;            // info/PADamage
  uint64_t attack_after = 0;  // 动作开始后多少毫秒结算判定

  SDL_FRect rect{0, 0, 0, 0};  // lt..rb（kRect / kArea 的单块区域）
  SDL_FPoint sp{0, 0};         // kTrapezoid 的攻击原点偏移
  int32_t range = 0;           // kTrapezoid 的射程 r

  int32_t start = 0;         // kArea 起始区域
  int32_t area_count = 0;    // kArea 区域数（上限 32）
  int32_t attack_count = 0;  // kArea 每次命中的区域个数
};

struct ServerMob {
  uint32_t index;
  std::u16string id;

  SDL_FPoint pos;

  std::u16string action;

  // 一次性动作（攻击 / 受击）播放中：期间不再重新决策
  bool one_shot = false;
  uint64_t action_end = 0;  // 一次性动作结束时刻

  int64_t hp;
  int64_t mp;

  uint8_t page;
  bool flip;

  uint64_t revive;  // 复活时间

  int32_t rx0;
  int32_t rx1;
  int32_t fh;

  float hspeed;
  float hspeed_max;
  float vspeed;

  float hforce = 0;

  uint64_t hate_id;

  std::flat_map<uint64_t, MobHit> hits;

  std::flat_map<GameDebuff::DebuffType, ServerDebuff> debuffs;

  bool create = false;  // 是否是道具召唤的

  // ---- 移动 / 动作状态 ----
  // 移动能力，与 v95 CMobTemplate::RegisterMob 的推导一致：
  // 0 不动 / 1 走 / 3 走+跳 / 4 飞 / 6 护送
  uint8_t move_ability = 1;
  int8_t input_x = 0;       // 移动输入 -1 / 0 / 1
  uint64_t target_id = 0;   // 正在追击的玩家（client_id）
  uint64_t think = 0;       // 下次巡游换向时刻
  uint64_t chase_end = 0;   // 放弃追击时刻
  uint64_t attack_cd = 0;   // 下次允许攻击的时刻
  uint64_t attack_hit = 0;  // 本次攻击的判定时刻，0 表示没有待结算攻击
  int32_t attack_index = 0;
  float fly_speed = 0;

  MobInterrupt interrupt;  // 被打断位移（击退 / 技能推动）
};

}  // namespace sdlms
