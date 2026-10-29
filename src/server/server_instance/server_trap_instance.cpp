#include "src/server/server_instance/server_trap_instance.h"

#include "src/client/game/game_effect.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_scene_instance.h"

namespace sdlms {

void ServerTrapInstance::HandleAttack(uint64_t client_id,
                                      fbs::ClientTrapAttackT& r) {
  if (!ServerClientInstance::clients_.contains(client_id)) {
    return;
  }
  // 转发
  auto map_id = r.map_id;
  auto clients = ServerSceneInstance::scenes_.at(map_id).clients;
  clients.erase(client_id);
  fbs::ServerTrapAttackT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (auto c : clients) {
    ServerResponse::SendToClient(c, t);
  }
}

void ServerTrapInstance::HandleSAttack(uint64_t client_id, fbs::AttackT& at) {
  const auto& attack = at;
  // 伤害数字
  DamageData data = {
      .num = attack.num,
      .type = (DamageData::DamageColor)attack.type,
  };
  GameEffect d = {
      .id = u"",
      .index = 0,
      .time = 0,
      .delay = attack.delay,
      .type = GameEffect::EffectType::kDamage,
      .pos = SDL_FPoint{attack.x, attack.y - 10},
      .z = true,
      .flip = false,
      .data = data,
  };
  EffectGameInstance::data_[7].emplace_back(d);
}

void ServerTrapInstance::HandleServerAttack(uint64_t client_id,
                                            fbs::ServerTrapAttackT& r) {
  HandleSAttack(client_id, *r.payload);
}

}  // namespace sdlms
