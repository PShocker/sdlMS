#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_mob.h"
#include "src/server/server/server_scene.h"

#include <wz/Node.h>

#include <cstdint>
#include <string>
#include <vector>

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

  // ---- Mob.wz 数据查询（带缓存） ----
  static wz::Node* LoadMobNode(const std::u16string& mob_id);
  static int32_t LoadMobInfoInt(const std::u16string& mob_id,
                                const char16_t* key, int32_t def);
  static bool HasMobAction(const std::u16string& mob_id,
                           const std::u16string& action);
  // 动作缺失时的回退：stand -> fly（与 v95 CMob::GetFineAction 一致）
  static std::u16string LoadFineAction(const std::u16string& mob_id,
                                       const std::u16string& action);
  // 动作总时长 = 各帧 delay 之和（zigzag 动作为往返序列的总和）
  static uint64_t LoadActionDelay(const std::u16string& mob_id,
                                  const std::u16string& action);
  // 渲染半宽（v95 CMob::GetHalfWidth：动作第二帧画布宽度的一半）
  static float LoadMobHalfWidth(const std::u16string& mob_id);
  static const std::vector<MobAttack>& LoadMobAttacks(
      const std::u16string& mob_id);

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
  static void HandleServerAttackPlayer(const fbs::ServerMobAttackPlayerT& m);
  static void HandleServerEvent(const fbs::ServerMobEventT& m);

  static void HandleDebuff(uint64_t client_id, fbs::ClientMobDebuffT& r);
  static void HandleSDebuff(fbs::ServerMobDebuffT& r);
};

}  // namespace sdlms
