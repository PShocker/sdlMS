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
  float knockback = 0.0f;
  float knockback_speed = 1.0f;  // Rush 等技能传 >1 的推进速度倍率
  bool hit_reaction = true;  // false: 命中已处于 hit 状态，只结算伤害
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
  bool fixed_distance = false;  // 固定距离击退，到达前不 snap
};

// Mob.wz/<id>.img/attackN/info 的解析结果，对应 v95 的 MobAttackInfo
struct MobAttack {
  // 攻击范围形状，对应 v95 LoadAttackInfo 按 info/type 展开的三种 range
  enum class RangeType {
    kRect,       // info/range/{lt,rb}        矩形判定
    kTrapezoid,  // info/range/{sp,r}          梯形判定（纵向半高 = 水平距离 / 4）
    kArea,       // info/range/{start,areaCount,attackCount,lt,rb} 区域连击
  };

  // v95 的 info/type，决定判定方式与攻击条目/子弹的生成方式
  enum NType : int32_t {
    kMelee = 0,       // 近战矩形（只在判定时刻做一次矩形求交）
    kRadius = 1,      // 欧氏距离（DoAttack 时已筛过一次，命中不再判矩形）
    kBullet = 2,      // 远程子弹（飞行到点，逐帧点判定）
    kAreaGround = 3,  // 区域攻击，每块区域按 foothold 找地面高度
    kAreaSelf = 4,    // 区域攻击，每块区域直接用自身 y
  };

  std::u16string action;  // 动作名 attackN
  int32_t index = 0;      // 攻击序号 i（0..8，v95 GetAttackInfo 的下标）

  RangeType type = RangeType::kRect;  // range 结构形状
  int32_t ntype = kMelee;             // v95 info/type（缺省时按 range 推断）

  bool magic = false;
  bool do_first = false;
  bool inactive = false;
  bool jump_attack = false;     // info/jumpAttack：只打空中的玩家
  bool deadly = false;          // info/deadlyAttack：把目标打到剩 1 HP
  bool mp_burn = false;         // info/mpBurn：魔法伤害强制 1
  bool knockback = false;       // info/knockback：命中强制击退
  bool rush = false;            // info/rush：冲锋型攻击
  bool tremble = false;         // info/tremble：命中震屏
  bool special_attack = false;  // info/specialAttack：需要蓄力就绪
  bool has_ball = false;        // info/ball 存在才生成子弹特效

  int32_t pad = 0;                // info/PADamage
  int32_t con_mp = 0;             // info/conMP：发动所需 MP
  int32_t element = 0;            // info/elemAttr：魔法元素（保留用）
  int32_t bullet_speed = 100;     // info/bulletSpeed，缺省 100
  int32_t bullet_number = 1;      // info/bulletNumber，>1 才算多弹
  int32_t rand_delay_attack = 0;  // info/randDelayAttack：区域攻击每块的随机延迟上限
  uint64_t attack_after = 0;      // 动作开始后多少毫秒结算判定
  uint64_t effect_after = 0;      // 动作开始后多少毫秒播放特效

  SDL_FRect rect{0, 0, 0, 0};  // lt..rb（kRect / kArea 的单块区域）
  SDL_FPoint sp{0, 0};         // kTrapezoid / kBullet 的攻击原点偏移
  int32_t range = 0;           // kTrapezoid / kBullet 的射程 r

  int32_t start = 0;         // kArea 起始区域
  int32_t area_count = 0;    // kArea 区域数（上限 32）
  int32_t attack_count = 0;  // kArea 每次命中的区域个数

  // v95 LoadAttackInfo 里的资源 UOL（存的是 wz 路径，客户端按帧加载）
  std::u16string effect_path;        // info/effect
  std::u16string hit_path;           // info/hit
  std::u16string ball_path;          // info/ball
  std::u16string area_warning_path;  // info/areaWarning
};

// v95 DoAttack 生成的"待结算攻击条目"（ATTACKENTRY）。
// 单体攻击用 attack_hit/attack_index；区域攻击每块区域各有自己的判定时刻，
// 子弹则在飞行途中逐帧做点判定，分别用下面的结构承载。
struct MobAttackTimer {
  uint64_t time = 0;         // 判定时刻（绝对毫秒）
  int32_t attack_index = 0;  // 攻击序号（LoadMobAttacks 的下标）
  int32_t block = -1;        // 区域列（v95 的 start + bit），-1 表示普通矩形
  float x = 0;               // 区域块中心 x（攻击发起时算好）
  float y = 0;               // 区域块地面 y
  bool left = false;         // 攻击朝向（v95 bLeft）
};

// v95 MobBullet：从攻击原点飞向瞄准点，飞行途中逐帧判断"点是否在目标身体内"
struct MobBullet {
  float x1 = 0;  // 起点
  float y1 = 0;
  float x2 = 0;  // 终点
  float y2 = 0;
  uint64_t start = 0;  // 出现/开始判定时刻（tAttackAfter）
  uint64_t end = 0;    // 到达终点时刻（start + tFlight）
  int32_t attack_index = 0;
  bool left = false;
};

// Mob.wz/<id>.img/info/skill/<n>：模板侧技能引用（v95 MobSkillInfo）
struct MobSkillRef {
  int32_t skill_id = 0;  // info/skill/<n>/skill
  int32_t slv = 1;       // info/skill/<n>/level
  int32_t action = 1;    // info/skill/<n>/action（1 基，动作名 "skill"+action）
  uint64_t effect_after = 0;  // info/skill/<n>/effectAfter
  uint64_t after_delay = 0;   // info/skill/<n>/afterDelay（客户端不读，服务端当间隔）
};

// Skill/MobSkill.img/<id>/level/<slv>：技能等级数据（v95 MOBSKILLLEVELDATA）
struct MobSkillLevel {
  int32_t hp_below = 0;   // hp：触发 HP 百分比阈值
  int32_t mp_con = 0;     // mpCon
  int32_t prop = 0;       // 触发概率 %
  int32_t x = 0;          // 主数值（伤害%/治疗量/力度…）
  int32_t y = 0;          // 次数值
  uint64_t duration = 0;  // time × 1000
  uint64_t interval = 0;  // interval × 1000（冷却）
  int32_t count = -1;     // 最多命中目标数，-1 = 不限
  bool random_target = false;
  int32_t direction = 0;
  int32_t elem = 0;
  int32_t limit = 0;  // 累计施放次数上限
  SDL_FRect area{0, 0, 0, 0};       // lt..rb（相对怪物原点）
  std::vector<int32_t> summon_ids;  // level 下 "0","1"… 召唤模板 id
  uint64_t anim_ms = 0;             // 动作总时长（Σdelay，用于结算时刻兜底）
  bool valid = false;               // wz 里是否存在该等级数据
};

// 护送路线节点（v95 opcode 0x130 CMob::OnEscortFullPath 的节点）
struct MobEscortNode {
  float x = 0;
  float y = 0;
  uint64_t stop_ms = 0;  // nStopDuration：到点后停留多久
};

struct ServerMob {
  uint32_t index = 0;
  std::u16string id;

  SDL_FPoint pos = {0.0f, 0.0f};
  SDL_FPoint spawn_pos = {0.0f, 0.0f};  // WZ 初始出生位置，复活回到这里
  int32_t spawn_fh = 0;
  uint8_t spawn_page = 0;

  std::u16string action;

  // 一次性动作（攻击 / 受击）播放中：期间不再重新决策
  bool one_shot = false;
  uint64_t action_end = 0;  // 一次性动作结束时刻

  int64_t hp = 0;
  int64_t mp = 0;
  // 已执行 RunDie：避免重复死亡事件，并让死亡动画期间停止物理下落
  bool dead = false;

  uint8_t page = 0;
  bool flip = false;

  uint64_t revive = 0;  // 死亡后下一次复活/渐显结束的时间
  bool reviving = false;  // 已复活，正在 1 秒渐显阶段

  int32_t rx0 = 0;
  int32_t rx1 = 0;
  int32_t fh = 0;

  float hspeed = 0.0f;
  float hspeed_max = 0.0f;
  float vspeed = 0.0f;

  float hforce = 0;

  std::flat_map<uint64_t, MobHit> hits;

  std::flat_map<GameDebuff::DebuffType, ServerDebuff> debuffs;

  bool create = false;  // 是否是道具召唤的

  // ---- 移动 / 动作状态 ----
  // 移动能力，与 v95 CMobTemplate::RegisterMob 的推导一致：
  // 0 不动 / 1 走 / 3 走+跳 / 4 飞 / 6 护送
  uint8_t move_ability = 1;
  int8_t input_x = 0;       // 移动输入 -1 / 0 / 1
  int8_t input_y = 0;       // 爬梯 / 绳的纵向输入 -1 / 0 / 1
  uint64_t target_id = 0;   // 正在追击的玩家（client_id）
  uint64_t think = 0;       // 下次巡游换向时刻
  uint64_t chase_end = 0;   // 放弃追击时刻
  uint64_t attack_cd = 0;   // 下次允许攻击的时刻
  uint64_t attack_hit = 0;  // 本次攻击的判定时刻，0 表示没有待结算攻击
  int32_t attack_index = 0;
  float fly_speed = 0;

  // 攻击条目的待结算列表（区域攻击每块一条，其余攻击走 attack_hit）
  std::vector<MobAttackTimer> attack_timers;
  // 在飞子弹
  std::vector<MobBullet> bullets;
  uint64_t last_area_attack = 0;  // v95 m_tLastAreaAttack：区域攻击硬冷却
  uint32_t area_mask = 0;         // v95 TARGETINFO.dwTargetInfo 的区域位掩码

  // ---- 被"迷惑"（v95 m_stat.nDazzle_ / Dazzle）----
  // nDazzle_ 存的是施法者角色 ID：0 表示未被迷惑
  uint64_t dazzle_owner = 0;
  uint64_t dazzle_end = 0;      // 迷惑到期时刻
  uint64_t dazzle_cd = 0;       // 下次允许身体撞击的时刻（v95 m_tLastHitDazzledMob）
  uint64_t hit_by_mob = 0;      // 下次允许被怪打的时刻（v95 m_tLastHitByMob）
  uint32_t target_mob = 0;      // 迷惑怪追击的怪（v95 m_pvcActive->m_pTarget）
  bool self_destroyed = false;  // 自爆已触发（v95 bFirstSelfDestruction 只触发一次）
  bool attack_ready = false;    // v95 m_bAttackReady：bSpecialAttack 的蓄力就绪

  // ---- 怪物技能（v95 info/skill + MobSkill.img）----
  uint64_t skill_hit = 0;     // 本次技能的结算时刻，0 表示没有待结算技能
  int32_t skill_index = 0;    // 本次技能在 LoadMobSkills 里的下标
  int32_t skill_id = 0;       // 技能 id（MobSkill.img 的键）
  int32_t skill_slv = 0;      // 技能等级
  std::flat_map<int32_t, uint64_t> skill_cd;     // 技能 id -> 下次可用时刻
  std::flat_map<int32_t, int32_t> skill_used;    // 技能 id -> 已施放次数（limit）

  // ---- 护送怪（moveAbility 6，v95 OnEscortFullPath）----
  // v95 的路线完全由服务器下发（opcode 0x130），客户端不做本地寻路；
  // 本工程由服务端自己生成/保存路线，这里存的就是服务器侧路线。
  int32_t escort_type = 0;                 // info/escort
  std::vector<MobEscortNode> escort_path;  // 航点表
  size_t escort_index = 0;                 // 当前航点
  uint64_t escort_stop = 0;                // 停留到此时刻（nStopDuration）

  // ---- 爬梯 / 绳（v95 rope 动作，动作表索引 4）----
  bool on_rope = false;
  int32_t rope_id = 0;
  float rope_x = 0;

  MobInterrupt interrupt;  // 被打断位移（击退 / 技能推动）
};

}  // namespace sdlms
