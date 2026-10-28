#ifndef SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_MOB_SYSTEM_H_
#define SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_MOB_SYSTEM_H_

#include "src/client/game/game_foothold.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_mob.h"
#include <cstdint>
#include <string>
#include <vector>
namespace sdlms {



class ServerMobSystem {
private:
  static inline uint32_t map_id_ = 0;
  static inline uint32_t delta_time_ = 0;
  static inline std::flat_map<int32_t, GameFoothold> g_fhs_;
  static inline fbs::ServerMobEventT events_;

  static int LoadMobHitCd(ServerMob &mob);

  static bool RunTryJump(ServerMob &mob);

  static bool RunHitCheck(ServerMob &mob);

  static bool RunDebuffAction(ServerMob &mob);

  static void RunHitAction(ServerMob &mob);
  static void RunDefaultAction(ServerMob &mob);
  static void RunMoveAction(ServerMob &mob);
  static void RunStandAction(ServerMob &mob);
  static void RunFlyAction(ServerMob &mob);
  static void RunSwimAction(ServerMob &mob);
  static void RunDieAction(ServerMob &mob);
  static void RunWalk(ServerMob &mob);
  static void RunStateMachine(ServerMob &mob);
  static void RunDuration(ServerMob &mob);
  static bool RunHitting(ServerMob &mob);
  static void RunHit(ServerMob &mob);
  static void RunDie(ServerMob &mob, uint64_t client_id);
  static void RunSend();
  static bool RunFall(ServerMob &mob);

  static void RunNetworkSyncPos(ServerMob &mob, ServerMob &o_mob);
  static void RunNetworkSyncFlip(ServerMob &mob, ServerMob &o_mob);
  static void RunNetworkSyncAction(ServerMob &mob, ServerMob &o_mob);
  static void RunNetworkSyncHp(ServerMob &mob, ServerMob &o_mob);

  static void RunNetworkSync(ServerMob &mob, ServerMob &o_mob);

public:
  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_MOB_SYSTEM_H_
