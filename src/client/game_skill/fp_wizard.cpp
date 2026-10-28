
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>

static void Huojianshu() {
  GameSkill g_skill;
  g_skill.id = u"2101003";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    GameTriangle tri = {{
        SDL_FPoint{-400, -100},
        SDL_FPoint{-400, 100},
        SDL_FPoint{0, -30},
    }};
    auto &sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAction(sf, u"shoot1");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      cm.data[0].hits = {60};
    }
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"210.img/skill/2101003/ball";
    auto pos = sf.pos;
    pos.y -= 30;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 400;
    } else {
      goal.x -= 400;
    }
    auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay,
                                                       page, 700, path);
    ClientRequest::send_to_host(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      // Create and send attack payload
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 2101003, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  Huojianshu();
  return true;
}();