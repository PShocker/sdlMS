
#include "src/client/game/game_ball.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"

#include <SDL3/SDL_rect.h>
#include <wz/Property.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace sdlms {

static void Mfdan() {
  GameSkill g_skill;
  g_skill.id = u"2001002";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    GameTriangle tri = {
        {SDL_FPoint{-400, -100}, SDL_FPoint{-400, 100}, SDL_FPoint{0, -30}}};
    auto& sf = CharacterGameInstance::self_;
    const std::array actions = {u"swingO1", u"swingO2", u"swingO3"};
    std::uniform_int_distribution<> dis(0, actions.size() - 1);
    auto& gen = RandomGameInstance::gen;
    CharacterLogicSystem::RunAction(sf, actions[dis(gen)]);
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      cm.data[0].hits = {60};
    }
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"200.img/skill/2001002/ball";
    auto pos = sf.pos;
    pos.y -= 30;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 400;
    } else {
      goal.x -= 400;
    }
    auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay, page,
                                                   700, path);
    ClientRequest::SendToHost(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      // Create and send attack payload
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d);
      ClientRequest::SendToHost(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 2001002, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}

static void Mfdun() {
  GameSkill g_skill;
  g_skill.id = u"2001000";
  g_skill.hit = [](GameMob* mob, uint64_t n) -> uint64_t {
    auto ski_lv = JobSkillGameInstance::LoadSkiLevel(u"2001000");
    auto x_node = WzResource::skill_->find(u"200.img/skill/2001000/level");
    auto x = static_cast<wz::Property<int>*>(
                 x_node->get_child(std::to_string(ski_lv))->get_child(u"x"))
                 ->get();
    return n * x;
  };
  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"2001000"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAction(sf, u"alert2");

    auto& ski = SkillGameInstance::ski_;

    g_skill.end();
    g_skill.lv = ski_lv;
    ski.push_back(g_skill);

    auto ckt = SkillGameInstance::CreateSkillPayload(2001000, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}

static void Mfsj() {
  GameSkill g_skill;
  g_skill.id = u"2001003";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunStandAction(sf);
    CharacterLogicSystem::RunAttackAction(sf, false);
    auto g_r = SkillGameInstance::LoadSkiR(u"2001003");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r);
    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      // Create and send attack payload
      cm.data[0].hits = {60, 70};
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay, 120, "",
                                                   "");
      ClientRequest::SendToHost(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 2001003, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}

static void Mfkaijia() {
  GameSkill g_skill;
  g_skill.id = u"2001001";

  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"2001001"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.effect = [](SDL_FPoint p, GameEffect* e, bool f) {
    const int duration = 300;
    auto start = e->delay;
    auto dt = Window::dt_now_ - start;
    if (dt <= duration) {
      float scale = 1 + 1.2 * (float)dt / duration;
      auto g_character = std::any_cast<GameCharacter*>(e->data);
      auto character = *g_character;
      character.scale = scale;
      character.color.a = 192 * (1 - (float)dt / duration);
      CharacterRenderSystem::RenderCharacter(character);
      return true;
    }
    return false;
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    auto& ski = SkillGameInstance::ski_;

    g_skill.end();
    g_skill.lv = ski_lv;
    ski.push_back(g_skill);

    CharacterLogicSystem::RunAction(sf, u"alert2");
    auto ckt = SkillGameInstance::CreateSkillPayload(2001001, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}

static void Molitisheng() {
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

[[maybe_unused]] static const bool r = [] {
  Mfdan();
  Mfsj();
  Mfkaijia();
  Molitisheng();
  return true;
}();

}  // namespace sdlms
