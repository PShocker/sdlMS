#include "mob_logic_system.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/request/client_request.h"

bool mob_logic_system::run() {
  auto visiable_mob = mob_game_instance::load_visible_mob();
  if (!visiable_mob.empty()) {
    client_request::client_visible_mob_request({
        .mob_ids = visiable_mob,
    });
  }
  return true;
}