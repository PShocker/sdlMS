#ifndef SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_MOB_INSTANCE_H_
#define SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_MOB_INSTANCE_H_

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_mob.h"
#include "src/server/server/server_scene.h"

#include <cstdint>
#include <string>

namespace sdlms {

class ServerMobInstance {
 public:
  struct MobDrop {
    std::u16string id;
    int min_quantity;
    int max_quantity;
    float rate;
  };
  static std::vector<MobDrop> LoadMobDrops(const std::u16string& mob_id);

  static std::vector<fbs::DropT> CreateMobDts(const std::u16string& mob_id,
                                              float x, float y, int page);

  static void LoadDefaultMob(const std::u16string id, ServerMob& mob);
  static void LoadMob(ServerSceneState& scene);

  static void HandleCreateMob(fbs::ClientCreateMobT& r);
  static void HandleSCreateMob(fbs::ServerCreateMobT& r);

  static void HanleServerMob(const std::unique_ptr<fbs::MobT>& m);

  static void HandleSAttack(uint64_t client_id, fbs::AttackT& at);

  static void HandleAttack(uint64_t client_id, fbs::ClientMobAttackT& r);
  static void HandleServerAttack(fbs::ServerMobAttackT& r);

  static void HandleServerMv(const fbs::ServerMobMvT& m);
  static void HandleServerFlip(const fbs::ServerMobFlipT& m);
  static void HandleServerAction(const fbs::ServerMobActionT& m);
  static void HandleServerState(const fbs::ServerMobStateT& m);
  static void HandleServerDie(const fbs::ServerMobDieT& m);
  static void HandleServerDrop(const fbs::ServerMobDropT& m);
  static void HandleServerEvent(const fbs::ServerMobEventT& m);

  static void HandleDebuff(uint64_t client_id, fbs::ClientMobDebuffT& r);
  static void HandleSDebuff(fbs::ServerMobDebuffT& r);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_MOB_INSTANCE_H_
