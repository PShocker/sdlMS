#include "src/client/system/logic/trap_logic_system.h"

#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_trap_instance.h"

namespace sdlms {

void TrapLogicSystem::RunMove(GameObj& g_trap) {
  auto node = WzResource::map_->find(g_trap.path);

  return;
}

bool TrapLogicSystem::RunAttackCheck(GameObj& g_trap) {
  return false;
}

void TrapLogicSystem::RunCollision() {
  if (CharacterLogicSystem::self_invincible_cooldown_ >= Window::dt_now_) {
    return;
  }
  auto& self = CharacterGameInstance::self_;
  if (self.action == u"dead") {
    return;
  }
  for (auto& m : TrapGameInstance::data_) {
    for (auto& g_trap : m | std::views::values) {
      if (RunAttackCheck(g_trap)) {
        uint64_t hit_num = 1;
        CharacterLogicSystem::RunBeingHit(g_trap.pos.x, hit_num);
        fbs::AttackT at{
            .delay = Window::dt_time_,
            .num = hit_num,
            .x = self.pos.x,
            .y = self.pos.y - 30,
            .type = static_cast<uint8_t>(DamageData::DamageColor::kWhite),
        };
        ServerTrapInstance::HandleSAttack(0, at);
        fbs::ClientTrapAttackT cta;
        cta.map_id = SceneSystemInstance::map_id_;
        cta.payload = std::make_unique<fbs::AttackT>(std::move(at));
        ClientRequest::SendToHost(cta);
        return;
      }
    }
  }
  return;
}

bool TrapLogicSystem::Run() {
  for (auto& o : TrapGameInstance::data_) {
    for (auto& g_trap : o | std::views::values) {
      ObjLogicSystem::RunAnimate(g_trap);
      ObjLogicSystem::RunAlpha(g_trap);
    }
  }
  RunCollision();
  return true;
}

}  // namespace sdlms
