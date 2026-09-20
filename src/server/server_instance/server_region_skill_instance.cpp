#include "server_region_skill_instance.h"
#include "server_scene_instance.h"
#include "src/server/server/server_region_skill.h"

void server_region_skill_instance::handle_r(uint64_t client_id,
                                            ClientRSkillT &r) {
  auto map_id = r.map_id;
  auto &scene = server_scene_instance::scenes.at(map_id);
  for (const auto &v : r.payload) {
    server_region_skill srs;
    srs.client_id = client_id;
    srs.id = v->id;
  }
}