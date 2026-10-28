
#include <SDL3/SDL_rect.h>
#include "src/client/game/game_character.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <memory>
#include <string>
namespace sdlms {


static void Duanhunjian() {
  GameSkill g_skill;
  g_skill.id = u"3001001";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    GameTriangle tri = {
        {
            SDL_FPoint{-350, -100},
            SDL_FPoint{-350, 100},
            SDL_FPoint{0, -28},
        },
    };
    auto &sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAction(sf, u"swingO1");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"300.img/skill/3001001/ball";
    auto pos = sf.pos;
    pos.y -= 28;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 350;
    } else {
      goal.x -= 350;
    }
    auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay,
                                                       page, 700, path);
    ClientRequest::send_to_host(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      // Create and send attack payload
      cm.data[0].y = 0;
      cm.data[0].hits = {10};
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 3001001, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

static void Shuangfeizhan() {
  GameSkill g_skill;
  g_skill.id = u"3001002";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    const GameTriangle tri = {
        {
            SDL_FPoint{-350, -100},
            SDL_FPoint{-350, 100},
            SDL_FPoint{0, -28},
        },
    };
    auto &sf = CharacterGameInstance::self_;
    auto WeaponType = EquipGameInstance::LoadWeaponType(sf);
    if (!(WeaponType == EquipGameInstance::WeaponType::kBow ||
          WeaponType == EquipGameInstance::WeaponType::kCrossbow)) {
      return;
    }
    auto ball = PackageGameInstance::LoadActiveBall();
    if (ball == nullptr) {
      return;
    }
    auto ball_id = (*ball)->id;
    CharacterLogicSystem::RunAction(sf, u"shoot1");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto pre = ball_id.substr(0, 4);
    std::u16string path = BallGameInstance::LoadBallPath(ball_id);
    auto pos = sf.pos;
    pos.y -= 32;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 350;
    } else {
      goal.x -= 350;
    }
    auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay,
                                                       page, 750, path);
    if (cct.payload->ball->mob_index != UINT32_MAX) {
      if (cct.payload->ball->y2 >= -8) {
        cct.payload->ball->y2 -= 8;
        cm.data[0].y -= 8;
      }
    }

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      // Create and send attack payload
      cm.data[0].hits = {5, 5};
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d);
      ClientRequest::send_to_host(cat);
    }
    for (auto i : {0, 1}) {
      cct.payload->ball->y1 = sf.pos.y - 28;
      cct.payload->ball->y2 += i * 8;
      ClientRequest::send_to_host(cct);
      ServerBallInstance::HandleServerB(cct.payload);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 3001002, ski_lv);
    if (!ckt.payload.empty()) {
      ckt.payload[1]->y += 8;
    }
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  Duanhunjian();
  Shuangfeizhan();
  return true;
}();

}  // namespace sdlms
