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

void trap_logic_system::run_move(game_obj &g_trap) {
  auto node = wz_resource::map->find(g_trap.path);

  return;
}

bool trap_logic_system::run_attack_check(game_obj &g_trap) { return false; }

void trap_logic_system::run_collision() {
  if (character_logic_system::self_invincible_cooldown >= window::dt_now) {
    return;
  }
  auto &self = character_game_instance::self;
  if (self.action == u"dead") {
    return;
  }
  for (auto &m : trap_game_instance::data) {
    for (auto &g_trap : m | std::views::values) {
      if (run_attack_check(g_trap)) {
        uint64_t hit_num = 1;
        character_logic_system::run_being_hit(g_trap.pos.x, hit_num);
        AttackT at{
            .delay = window::dt_time,
            .num = hit_num,
            .x = self.pos.x,
            .y = self.pos.y - 30,
            .type = damage_data::v,
        };
        server_trap_instance::handle_s_attack(0, at);
        ClientTrapAttackT cta;
        cta.map_id = scene_system_instance::map_id;
        cta.payload = std::make_unique<AttackT>(std::move(at));
        client_request::send_to_host(cta);
        return;
      }
    }
  }
  return;
}

bool trap_logic_system::run() {
  for (auto &o : trap_game_instance::data) {
    for (auto &g_trap : o | std::views::values) {
      obj_logic_system::run_animate(g_trap);
      obj_logic_system::run_alpha(g_trap);
    }
  }
  run_collision();
  return true;
}