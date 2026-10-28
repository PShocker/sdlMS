
#include <SDL3/SDL_rect.h>
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <vector>
namespace sdlms {


// 310
static void Guanchuanjian() {
  GameSkill g_skill;
  g_skill.id = u"3201004";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAttackAction(sf, true);
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"320.img/skill/3201004/ball";
    auto pos = sf.pos;
    pos.y -= 30;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    const auto w = 400;
    const auto h = 72;
    if (sf.flip) {
      goal.x += w;
    } else {
      goal.x -= w;
    }
    CheckMobs cm;
    auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay,
                                                       page, 800, path);
    ClientRequest::send_to_host(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    SDL_FPoint lt{-w, -h};
    SDL_FPoint rb{0, 0};
    auto g_r = SkillGameInstance::LoadR(lt, rb, sf.pos, sf.flip);
    cm = CharacterLogicSystem::RunAttackCheck(sf, g_r);
    if (!cm.data.empty()) {
      for (auto &data : cm.data) {
        cct.payload->ball->mob_index = data.mob.index;
        auto mob_pos = MobGameInstance::data_.at(data.mob.index).mob.pos;
        cct.payload->ball->y1 = mob_pos.y;
        cct.payload->ball->x2 = 0;
        cct.payload->ball->y2 = 0;
        data.delay = BallGameInstance::LoadBallTime(cct);
        data.hits = {100};
      }
      // Create and send attack payload
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, 0);
      for (auto &data : cat.payload) {
        data->effect = "Afterimage/hit.img/sword2";
      }
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(4111004, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

static void Wuyingjian() {}

[[maybe_unused]] static const bool r = [] {
  Guanchuanjian();
  Wuyingjian();
  return true;
}();

}  // namespace sdlms
