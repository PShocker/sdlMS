#include "server_mob_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server/server_mob.h"
#include "wz/Property.h"
#include <optional>

void server_mob_instance::load_server_mob(uint32_t map_id) {
  server_common_map_mob mobs;
  auto data = mob_game_instance::load(map_id);
  for (auto &v : data) {
    for (auto &mob : v) {
      server_mob m_mob{
          .index = mob.index,
          .id = mob.id,
          .rx0 = mob.rx0,
          .rx1 = mob.rx1,
          .pos = mob.pos,
          .hate_id = std::nullopt,
          .hp = mob.hp,
          .mp = mob.mp,
          .action = mob.action,
          .hspeed = 0,
          .vspeed = 0,
          .fh = mob.fh,
      };
      mobs.mobs[mob.index] = m_mob;
    }
  }
  common_map_mob[map_id] = mobs;
}