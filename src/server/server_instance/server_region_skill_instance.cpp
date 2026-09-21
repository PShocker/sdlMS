#include "server_region_skill_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_region_skill.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/server/server/server_region_skill.h"
#include <format>

void server_region_skill_instance::handle_rskill(uint64_t client_id,
                                                 ClientRSkillT &r) {
  auto map_id = r.map_id;
  auto &scene = server_scene_instance::scenes.at(map_id);
  for (const auto &v : r.payload) {
    server_region_skill srs;
    srs.client_id = client_id;
    srs.id = v->id;
  }
}

void server_region_skill_instance::handle_s_rskill(ServerRSkillT &r) {
  for (const auto &v : r.payload) {
    game_region_skill grs;
    auto tmp = std::format("{:07d}", v->id);
    grs.id = {tmp.begin(), tmp.end()};
    grs.start = v->start;
    grs.end = v->end;
    grs.flip = v->flip;
    grs.lv = v->lv;
    grs.pos = {v->x, v->y};

    game_effect e = {
        .type = game_effect::effect_type::skill_region,
        .data = grs,
    };
    effect_game_instance::data[v->page].emplace_back(e);
  }
}