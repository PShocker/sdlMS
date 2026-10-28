#include "server_region_skill_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_region_skill.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/server/server/server_region_skill.h"
#include <format>
namespace sdlms {


void ServerRegionSkillInstance::HandleRskill(uint64_t client_id,
                                                 fbs::ClientRSkillT &r) {
  auto map_id = r.map_id;
  auto &scene = ServerSceneInstance::scenes_.at(map_id);
  for (const auto &v : r.payload) {
    ServerRegionSkill srs;
    srs.client_id = client_id;
    srs.id = v->ski_id;
  }
}

void ServerRegionSkillInstance::HandleSRskill(fbs::ServerRSkillT &r) {
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
        .type = GameEffect::EffectType::kSkillRegion,
        .data = grs,
    };
    EffectGameInstance::data_[v->page].emplace_back(e);
  }
}

}  // namespace sdlms
