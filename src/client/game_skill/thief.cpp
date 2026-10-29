
#include "src/client/game/game_character.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <memory>
#include <string>

namespace sdlms {

static void Yinshenshu() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"4001001";
  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"4001001"; });
    if (it != ski.end()) {
      ski.erase(it);
      auto& ccs = CharacterLogicSystem::ccs_;
      fbs::StateT st;
      st.state = fbs::StateEnum_BUFF_SKILL;
      st.val = 4001001;
      st.sub_val = 0;
      ccs.payload.push_back(std::make_unique<fbs::StateT>(st));
    }
  };

  g_skill.state = [](GameCharacter* c, int index) {
    if (index == 1) {
      c->color = {255, 255, 255, 128};
    } else {
      c->color = {255, 255, 255, 255};
    }
  };
  g_skill.frame = [g_skill]() {
    auto& self = CharacterGameInstance::self_;
    self.color = {255, 255, 255, 128};
    auto type = CharacterLogicSystem::LoadActionType(self);
    switch (type) {
      case CharacterLogicSystem::ActionEnum::kAttack:
      case CharacterLogicSystem::ActionEnum::kSkill: {
        g_skill.end();
        break;
      }
      default: {
        CharacterLogicSystem::self_invincible_cooldown_ = Window::dt_now_ + 100;
        break;
      }
    }
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    SkillGameInstance::Skis()[u"4001001"].cd = Window::dt_now_ + 1000;

    auto& ski = SkillGameInstance::ski_;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    ski.push_back(g_skill);

    auto& sf = CharacterGameInstance::self_;

    auto ckt = SkillGameInstance::CreateSkillPayload(4001001, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);

    auto& ccs = CharacterLogicSystem::ccs_;
    fbs::StateT st;
    st.state = fbs::StateEnum_BUFF_SKILL;
    st.val = 4001001;
    st.sub_val = 1;
    ccs.payload.push_back(std::make_unique<fbs::StateT>(st));

    g_skill.state(&sf, 0);
  };

  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}

static void Huibishu() {
  GameSkill g_skill;
  g_skill.id = u"2001099";

  g_skill.end = []() {
    auto& skills = SkillGameInstance::Skis();
    auto ski_lv = skills[u"2001099"].lv;
  };
  g_skill.passive = [](int ski_lv) {
    auto& skills = SkillGameInstance::Skis();
    skills[u"2001099"].lv = ski_lv;
  };
  auto& skills = SkillGameInstance::Skis();

  skills[g_skill.id] = g_skill;
}

static void Shuangfeizhan() {
  GameSkill g_skill;
  g_skill.id = u"4001003";
  g_skill.fall = true;
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    GameTriangle tri = {
        {
            SDL_FPoint{-350, -100},
            SDL_FPoint{-350, 100},
            SDL_FPoint{0, -28},
        },
    };
    auto& sf = CharacterGameInstance::self_;
    auto WeaponType = EquipGameInstance::LoadWeaponType(sf);
    if (WeaponType != EquipGameInstance::WeaponType::kClaw) {
      return;
    }
    auto ball = PackageGameInstance::LoadActiveBall();
    if (ball == nullptr) {
      return;
    }
    auto ball_id = (*ball)->id;
    CharacterLogicSystem::RunStandAction(sf);
    CharacterLogicSystem::RunAttackAction(sf, true);
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    std::u16string path = u"Consume/0207.img/" + ball_id + u"/bullet";
    auto pos = sf.pos;
    pos.y -= 28;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 350;
    } else {
      goal.x -= 350;
    }
    auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay, page,
                                                   700, path);
    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      // Create and send attack payload
      cm.data[0].hits = {15, 15};
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d, 120, "", "");
      ClientRequest::SendToHost(cat);
    }
    for (auto i : {0, 1}) {
      cct.payload->ball->y1 += i * 5;
      cct.payload->ball->y2 += i * 5;
      ClientRequest::SendToHost(cct);
      ServerBallInstance::HandleServerB(cct.payload);
      cct.payload->ball->delay += 120;
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 4001003, ski_lv);
    if (!ckt.payload.empty()) {
      ckt.payload[1]->y += 10;
    }
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}

static void Erlianji() {
  GameSkill g_skill;
  g_skill.id = u"4001002";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAttackAction(sf, false);
    auto g_r = SkillGameInstance::LoadSkiR(u"4001002");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r, 1);
    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      // Create and send attack payload
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay);
      ClientRequest::SendToHost(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 4001002, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  Yinshenshu();
  Huibishu();
  Shuangfeizhan();
  Erlianji();
  return true;
}();

}  // namespace sdlms
