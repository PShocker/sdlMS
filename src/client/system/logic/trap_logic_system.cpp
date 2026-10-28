#include "trap_logic_system.h"
#include "character_logic_system.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_trap_instance.h"

void TrapLogicSystem::run_move(GameObj &g_trap) {
  auto node = WzResource::map->find(g_trap.path);

  return;
}

bool TrapLogicSystem::run_attack_check(GameObj &g_trap) { return false; }

void TrapLogicSystem::run_collision() {
  if (CharacterLogicSystem::self_invincible_cooldown >= Window::dt_now) {
    return;
  }
  auto &self = CharacterGameInstance::self;
  if (self.action == u"dead") {
    return;
  }
  for (auto &m : TrapGameInstance::data) {
    for (auto &g_trap : m | std::views::values) {
      if (run_attack_check(g_trap)) {
        uint64_t hit_num = 1;
        CharacterLogicSystem::run_being_hit(g_trap.pos.x, hit_num);
        AttackT at{
            .delay = Window::dt_time,
            .num = hit_num,
            .x = self.pos.x,
            .y = self.pos.y - 30,
            .type = DamageData::v,
        };
        ServerTrapInstance::handle_s_attack(0, at);
        ClientTrapAttackT cta;
        cta.map_id = SceneSystemInstance::map_id;
        cta.payload = std::make_unique<AttackT>(std::move(at));
        ClientRequest::send_to_host(cta);
        return;
      }
    }
  }
  return;
}

bool TrapLogicSystem::run() {
  for (auto &o : TrapGameInstance::data) {
    for (auto &g_trap : o | std::views::values) {
      ObjLogicSystem::run_animate(g_trap);
      ObjLogicSystem::run_alpha(g_trap);
    }
  }
  run_collision();
  return true;
}