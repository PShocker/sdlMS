
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <memory>
#include <vector>

static void Erduantiao() {
  GameSkill g_skill;
  g_skill.fall = true;
  g_skill.id = u"4111005";
  g_skill.end = []() {};

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &ski = SkillGameInstance::ski;
    auto &sf = CharacterGameInstance::self;
    auto action_type = CharacterLogicSystem::LoadActionType(sf);
    if (action_type != CharacterLogicSystem::ActionEnum::kJump) {
      return;
    }
    CharacterLogicSystem::self_vspeed -= 420;
    if (sf.flip == 1) {
      // 朝右
      CharacterLogicSystem::self_hspeed += 480;
    } else {
      CharacterLogicSystem::self_hspeed -= 480;
    }
    CharacterLogicSystem::self_two_jump_cooldown = UINT64_MAX;
    auto ckt = SkillGameInstance::CreateSkillPayload(4111005, ski_lv);
    ckt.x = sf.pos.x;
    ckt.y = sf.pos.y;
    ckt.flip = sf.flip;
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0,
                                          ckt.x, ckt.y, ckt.flip);
    ClientRequest::send_to_host(ckt);
    SkillGameInstance::Skis()[u"4111005"].cd = Window::dt_now + 100;
  };

  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

static void Dafeibiao() {
  GameSkill g_skill;
  g_skill.id = u"4111004";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::RunAction(sf, u"avenger");
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"411.img/skill/4111004/ball";
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

    ClientCharacterAttackT cat;
    SDL_FPoint lt{-w, -h};
    SDL_FPoint rb{0, 0};
    auto g_r = SkillGameInstance::LoadR(lt, rb, sf.pos, sf.flip);
    cm = CharacterLogicSystem::RunAttackCheck(sf, g_r);
    if (!cm.data.empty()) {
      for (auto &data : cm.data) {
        cct.payload->ball->mob_index = data.mob.index;
        auto mob_pos = MobGameInstance::data.at(data.mob.index).mob.pos;
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

[[maybe_unused]] static const bool r = [] {
  Erduantiao();
  Dafeibiao();
  return true;
}();