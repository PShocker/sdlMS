#include "server_mob_instance.h"
#include "server_scene_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server/server_mob.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <optional>

std::flat_map<uint32_t, server_mob>
server_mob_instance::load_mob(uint32_t map_id) {
  std::flat_map<uint32_t, server_mob> r;
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
      r[mob.index] = m_mob;
    }
  }
  return r;
}

void server_mob_instance::load_vsb_mob(uint32_t m_id,
                                       std::vector<uint32_t> &mb_ids) {
  auto &mobs = server_scene_instance::scenes[m_id].mobs;
  for (auto m : mb_ids) {
    mobs[m].visible = true;
  }
}