#include "server_mob_instance.h"
#include "server_scene_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server/server_mob.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <optional>

void server_mob_instance::load_mob(server_scene &s_scene) {
  auto mobs = mob_game_instance::load(s_scene.map_id);
  for (auto ms : mobs) {
    for (auto m : ms) {
      s_scene.mobs[m.index] = server_mob{m};
    }
  }
}