
#include "SDL3/SDL_rect.h"
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
#include "wz/Property.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

static void mfdan() {
  GameSkill g_skill;
  g_skill.id = u"2001002";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    GameTriangle tri = {
        {SDL_FPoint{-400, -100}, SDL_FPoint{-400, 100}, SDL_FPoint{0, -30}}};
    auto &sf = CharacterGameInstance::self;
    const std::array actions = {u"swingO1", u"swingO2", u"swingO3"};
    std::uniform_int_distribution<> dis(0, actions.size() - 1);
    auto &gen = RandomGameInstance::gen;
    CharacterLogicSystem::run_action(sf, actions[dis(gen)]);
    auto cm = CharacterLogicSystem::run_attack_check(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      cm.data[0].hits = {60};
    }
    auto delay = SkillGameInstance::load_ski_time(sf);
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
    auto cct = BallGameInstance::create_ball_payload(cm, pos, goal, delay,
                                                       page, 700, path);
    ClientRequest::send_to_host(cct);
    ServerBallInstance::handle_server_b(cct.payload);

    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::load_ball_time(cct);
      // Create and send attack payload
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, d);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 2001002, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

static void mfdun() {
  GameSkill g_skill;
  g_skill.id = u"2001000";
  g_skill.hit = [](GameMob *mob, uint64_t n) -> uint64_t {
    auto ski_lv = JobSkillGameInstance::load_ski_level(u"2001000");
    auto x_node = WzResource::skill->find(u"200.img/skill/2001000/level");
    auto x = static_cast<wz::Property<int> *>(
                 x_node->get_child(std::to_string(ski_lv))->get_child(u"x"))
                 ->get();
    return n * x;
  };
  g_skill.end = []() {
    auto &ski = SkillGameInstance::ski;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill &s) { return s.id == u"2001000"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_action(sf, u"alert2");

    auto &ski = SkillGameInstance::ski;

    g_skill.end();
    g_skill.lv = ski_lv;
    ski.push_back(g_skill);

    auto ckt = SkillGameInstance::create_skill_payload(2001000, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

static void mfsj() {
  GameSkill g_skill;
  g_skill.id = u"2001003";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_stand_action(sf);
    CharacterLogicSystem::run_attack_action(sf, false);
    auto g_r = SkillGameInstance::load_ski_r(u"2001003");
    auto cm = CharacterLogicSystem::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      auto delay = SkillGameInstance::load_ski_time(sf);
      // Create and send attack payload
      cm.data[0].hits = {60, 70};
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, delay, 120,
                                                       "", "");
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 2001003, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

static void mfkaijia() {
  GameSkill g_skill;
  g_skill.id = u"2001001";

  g_skill.end = []() {
    auto &ski = SkillGameInstance::ski;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill &s) { return s.id == u"2001001"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.effect = [](SDL_FPoint p, GameEffect *e, bool f) {
    const int duration = 300;
    auto start = e->delay;
    auto dt = Window::dt_now - start;
    if (dt <= duration) {
      float scale = 1 + 1.2 * (float)dt / duration;
      auto g_character = std::any_cast<GameCharacter *>(e->data);
      auto character = *g_character;
      character.scale = scale;
      character.color.a = 192 * (1 - (float)dt / duration);
      CharacterRenderSystem::render_character(character);
      return true;
    }
    return false;
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &sf = CharacterGameInstance::self;
    auto &ski = SkillGameInstance::ski;

    g_skill.end();
    g_skill.lv = ski_lv;
    ski.push_back(g_skill);

    CharacterLogicSystem::run_action(sf, u"alert2");
    auto ckt = SkillGameInstance::create_skill_payload(2001001, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

static void molitisheng() {
  GameSkill g_skill;
  g_skill.id = u"2001099";

  g_skill.end = []() {
    auto &skis = SkillGameInstance::skis();
    auto ski_lv = skis[u"2001099"].lv;
  };
  g_skill.passive = [](int ski_lv) {
    auto &skis = SkillGameInstance::skis();
    skis[u"2001099"].lv = ski_lv;
  };
  auto &skis = SkillGameInstance::skis();

  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  mfdan();
  mfsj();
  mfkaijia();
  molitisheng();
  return true;
}();