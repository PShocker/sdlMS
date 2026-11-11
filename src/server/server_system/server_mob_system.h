#pragma once

#include "src/client/game/game_foothold.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_mob.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

namespace sdlms {

class ServerMobSystem {
 private:
  static inline uint32_t map_id_ = 0;
  static inline uint32_t delta_time_ = 0;
  static inline std::flat_map<int32_t, GameFoothold> g_fhs_;
  static inline fbs::ServerMobEventT events_;
  // 当前正在处理的场景怪物表（迷惑怪互殴 / 技能召唤需要访问其他怪）
  static inline std::flat_map<uint32_t, ServerMob>* mobs_ = nullptr;

  // 技能召唤：不能在遍历 scene.mobs 时插入，延迟到本帧末尾处理
  struct PendingSummon {
    uint32_t map_id = 0;
    float x = 0;
    float y = 0;
    uint8_t page = 0;
    int32_t fh = 0;
    std::vector<int32_t> mob_ids;
  };
  static inline std::vector<PendingSummon> pending_summons_;

  static void RunPendingSummons();

  // ---- 动作 ----
  static std::u16string LoadDefaultAction(const ServerMob& mob);
  static void RunSetAction(ServerMob& mob, const std::u16string& action);
  static void RunMoveAction(ServerMob& mob);

  // ---- 决策 ----
  static bool CanAttack(const ServerMob& mob);
  static void RunTarget(ServerMob& mob);
  static void RunInput(ServerMob& mob);
  static bool RunTryAttack(ServerMob& mob);
  static SDL_FRect LoadTargetRect(uint64_t client_id);
  static bool RunAttackRange(const ServerMob& mob, const MobAttack& attack,
                             const SDL_FRect& target);
  static void RunAttackHit(ServerMob& mob);

  // ---- 攻击结算（v95 DoAttack / ProcessAttack） ----
  // 把"怪物攻击玩家"的事件打包下发（伤害数值由客户端结算）
  static void RunSendMobDamage(ServerMob& mob, uint64_t client_id,
                               const MobAttack& attack);
  // 区域攻击：逐格 foothold 检测 + 2500ms 冷却 + 不重复随机抽 attackCount 格
  static bool RunAreaCandidate(const ServerMob& mob, const MobAttack& attack,
                               const SDL_FRect& target, uint32_t* mask);
  static bool LoadFootholdY(float x, float top, float bottom, float* y);
  // 区域攻击：按位掩码生成每块区域的攻击条目（各自带随机延迟）
  static void RunAreaEntries(ServerMob& mob, const MobAttack& attack,
                             int32_t attack_index, uint32_t mask);
  // 每帧处理到期的区域攻击条目
  static void RunAttackTimers(ServerMob& mob);
  // 子弹攻击（v95 MobBullet）：生成子弹 + 逐帧点判定
  static void RunBulletEntries(ServerMob& mob, const MobAttack& attack,
                               int32_t attack_index, const SDL_FRect& target);
  static void RunUpdateBullets(ServerMob& mob);

  // ---- 怪物技能（v95 TryDoingSkill + MobSkill.img）----
  static bool RunTrySkill(ServerMob& mob);
  static void RunSkillHit(ServerMob& mob);


  // ---- 迷惑（v95 Dazzle）：怪打怪 ----
  // 怪物身体矩形（近似：半宽来自 wz 画布，高 = 2×半宽）
  static SDL_FRect LoadMobRect(const ServerMob& mob);
  // 服务端权威的怪打怪伤害（v95 CalcDamage::PDamage(MobStat*, MobStat*)）
  static int64_t MobVsMobDamage(const ServerMob& attacker,
                                const ServerMob& victim, bool magic);
  // 被迷惑的怪每 2000ms 与"未被迷惑的怪"做一次身体撞击
  static void RunDazzleBodyAttack(ServerMob& mob);
  // 迷惑怪的追击目标改为最近的怪（v95 TryFirstAttack 的迷惑分支）
  static void RunDazzleTarget(ServerMob& mob);
  static void RunSendHitMob(ServerMob& mob, const ServerMob& victim,
                            int64_t damage);

  // 下发怪物特效（v95 sEffect / sHit / areaWarning / MobSkill effect）
  // kind 0 = Mob.wz，1 = Skill.wz；x/y 是世界坐标
  static void RunSendEffect(ServerMob& mob, uint8_t kind,
                            const std::u16string& path, float x, float y,
                            bool flip, uint64_t delay);
  // 自爆：模板 info/selfDestruction 非 0 时，攻击框与玩家相交即爆炸
  static void RunTrySelfDestruction(ServerMob& mob);
  // bSpecialAttack 的蓄力就绪（v95 CP_MobChargeCount → m_bAttackReady）
  static void RunSetAttackReady(ServerMob& mob, bool ready);
  // 给玩家挂状态（v95 怪物技能的元素效果）：广播 ServerCharacterState(DEBUFF)
  static void RunSendPlayerDebuff(uint64_t client_id,
                                  GameDebuff::DebuffType type, int64_t val,
                                  uint64_t duration_ms);

  // ---- 护送（moveAbility 6）----
  // 没有路线时返回 false（v95：没有路径不巡游，原地不动）
  static bool RunEscort(ServerMob& mob);

  // ---- 爬梯 / 绳（v95 rope 动作）----
  static bool RunTryRope(ServerMob& mob);
  static void RunRope(ServerMob& mob);

  // ---- 物理 ----
  static void RunWalk(ServerMob& mob);
  static bool RunFall(ServerMob& mob);
  static void RunFly(ServerMob& mob);
  static void RunTryJump(ServerMob& mob);
  static void RunPhysics(ServerMob& mob);

  // ---- 被打断位移：v95 CVecCtrlMob::UpdateActiveInterrupted ----
  static void RunInterruptStart(ServerMob& mob, int32_t type, float end_x,
                                float end_y, uint32_t t_elapse);
  static void RunInterruptStep(ServerMob& mob, uint32_t t_elapse);
  static void RunInterruptGround(ServerMob& mob, uint32_t t_elapse);
  static void RunInterruptFlight(ServerMob& mob, uint32_t t_elapse);

  // ---- 受击 / 死亡 ----
  static bool RunHitting(ServerMob& mob);
  static bool RunHitCheck(ServerMob& mob);
  static void RunHitAction(ServerMob& mob);
  static void RunDot(ServerMob& mob);
  static void RunDie(ServerMob& mob, uint64_t client_id);

  // ---- 同步 ----
  static void RunNetworkSyncPos(ServerMob& mob, ServerMob& o_mob);
  static void RunNetworkSyncFlip(ServerMob& mob, ServerMob& o_mob);
  static void RunNetworkSyncAction(ServerMob& mob, ServerMob& o_mob);
  static void RunNetworkSyncHp(ServerMob& mob, ServerMob& o_mob);
  static void RunNetworkSync(ServerMob& mob, ServerMob& o_mob);
  static void RunSend();

  static void RunStateMachine(ServerMob& mob);

 public:
  // 给护送怪设置一条往返路线（本工程由服务端生成，v95 由服务器 0x130 下发）
  static void RunSetEscortPath(ServerMob& mob, float x1, float x2, float y,
                               uint64_t stop_ms = 0);
  // 只下发一次伤害事件（技能/怪打怪共用）
  static void RunSendDamageValue(ServerMob& mob, uint64_t client_id,
                                 int64_t damage, float x, float y);

  // 按 v95 的 nMoveType 档位把怪物推向 (end_x, end_y)，技能推/拉可复用。
  // type 取值与初速/阻力/停止档位的对应关系见 RunInterruptStart。
  static void RunInterrupt(ServerMob& mob, int32_t type, float end_x,
                           float end_y);
  static void RunInterruptDistance(ServerMob& mob, float distance, bool left,
                                   float speed_scale = 1.0f);

  static bool Run();
};

}  // namespace sdlms
