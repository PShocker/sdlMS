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
  // 按 v95 的 nMoveType 档位把怪物推向 (end_x, end_y)，技能推/拉可复用。
  // type 取值与初速/阻力/停止档位的对应关系见 RunInterruptStart。
  static void RunInterrupt(ServerMob& mob, int32_t type, float end_x,
                           float end_y);

  static bool Run();
};

}  // namespace sdlms
