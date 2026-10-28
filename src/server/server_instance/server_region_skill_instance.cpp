#include "server_region_skill_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_region_skill.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/server/server/server_region_skill.h"
#include <format>

void ServerRegionSkillInstance::handle_rskill(uint64_t client_id,
                                                 ClientRSkillT &r) {
  auto map_id = r.map_id;
  auto &scene = ServerSceneInstance::scenes.at(map_id);
  for (const auto &v : r.payload) {
    ServerRegionSkill srs;
    srs.client_id = client_id;
    srs.id = v->ski_id;
  }
}

void ServerRegionSkillInstance::handle_s_rskill(ServerRSkillT &r) {
  for (const auto &v : r.payload) {
    GameRegionSkill grs;
    auto tmp = std::format("{:07d}", v->ski_id);
    grs.id = {tmp.begin(), tmp.end()};
    grs.start = v->start;
    grs.end = v->end;
    grs.flip = v->flip;
    grs.lv = v->ski_lv;
    grs.pos = {v->x, v->y};

    GameEffect e = {
        .type = GameEffect::EffectType::skill_region,
        .data = grs,
    };
    EffectGameInstance::data[v->page].emplace_back(e);
  }
}