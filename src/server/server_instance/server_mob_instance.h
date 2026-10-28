#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_mob.h"
#include "src/server/server/server_scene.h"
#include <cstdint>
#include <string>
namespace sdlms {


using namespace fbs;

class ServerMobInstance {
public:
  struct MobDrop {
    std::u16string id;
    int min_quantity;
    int max_quantity;
    float rate;
  };
  static std::vector<MobDrop> LoadMobDrops(const std::u16string &mob_id);

  static std::vector<DropT> CreateMobDts(const std::u16string &mob_id,
                                           float x, float y, int page);

  static void LoadDefaultMob(const std::u16string id, ServerMob &mob);
  static void LoadMob(ServerSceneState &scene);

  static void HandleCreateMob(ClientCreateMobT &r);
  static void HandleSCreateMob(ServerCreateMobT &r);

  static void HanleServerMob(const std::unique_ptr<fbs::MobT> &m);

  static void HandleSAttack(uint64_t client_id, AttackT &at);

  static void HandleAttack(uint64_t client_id, ClientMobAttackT &r);
  static void HandleServerAttack(ServerMobAttackT &r);

  static void HandleServerMv(const ServerMobMvT &m);
  static void HandleServerFlip(const ServerMobFlipT &m);
  static void HandleServerAction(const ServerMobActionT &m);
  static void HandleServerState(const ServerMobStateT &m);
  static void HandleServerDie(const ServerMobDieT &m);
  static void HandleServerDrop(const ServerMobDropT &m);
  static void HandleServerEvent(const ServerMobEventT &m);

  static void HandleDebuff(uint64_t client_id, ClientMobDebuffT &r);
  static void HandleSDebuff(ServerMobDebuffT &r);
};

}  // namespace sdlms
