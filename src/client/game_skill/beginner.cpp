
#include "src/client/game/game_effect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_mob_instance.h"

#include <SDL3/SDL_rect.h>
#include <wz/Property.h>

#include <cstdint>
#include <format>
#include <string>
#include <vector>

namespace sdlms {

static int LoadSkiInt(const std::u16string& id, int lv, const char16_t* key,
                      int def) {
  auto* node =
      SkillGameInstance::LoadSkiLevelNode(id, static_cast<uint8_t>(lv));
  if (node == nullptr) {
    return def;
  }
  auto* child = node->get_child(key);
  if (child == nullptr) {
    return def;
  }
  if (child->type == wz::Type::Int) {
    return static_cast<wz::Property<int>*>(child)->get();
  }
  if (child->type == wz::Type::String) {
    auto text = static_cast<wz::Property<std::u16string>*>(child)->get();
    try {
      return std::stoi(std::string(text.begin(), text.end()));
    } catch (...) {
      return def;
    }
  }
  return def;
}

static std::u16string LoadShellId(int lv) {
  auto id = LoadSkiInt(u"0001000", lv, u"itemCon", 0);
  auto text = std::to_string(id);
  if (text.size() < 8) {
    text.insert(0, 8 - text.size(), '0');
  }
  return {text.begin(), text.end()};
}

static void ThreeSnails() {
  GameSkill g_skill;
  g_skill.id = u"0001000";
  g_skill.can_use = [](int ski_lv) { return true; };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    GameTriangle tri = {{
        SDL_FPoint{-350, -100},
        SDL_FPoint{-350, 100},
        SDL_FPoint{0, -28},
    }};
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAttackAction(sf, false, true);
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"000.img/skill/0001000/level/";
    path += {ski_lvl2.begin(), ski_lvl2.end()};
    path += u"/ball";
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
    ClientRequest::SendToHost(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      // Create and send attack payload
      cm.data[0].y = 0;
      cm.data[0].hits = {LoadSkiInt(u"0001000", ski_lv, u"fixdamage", 1)};
      cm.data[0].raw_damage = true;
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d);
      ClientRequest::SendToHost(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 1000, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}

static void Recovery() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"0001001";

  static uint64_t now;
  static int heal_amount = 0;
  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"0001001"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.frame = []() {
    if (now <= Window::dt_now_) {
      const auto& pos = CharacterGameInstance::self_.pos;
      now = Window::dt_now_ + 5000;
      fbs::AttackT at;
      at.x = pos.x - 10;
      at.y = pos.y - 30;
      at.type = static_cast<uint8_t>(DamageData::DamageColor::kBlue);
      at.num = heal_amount;
      ServerMobInstance::HandleSAttack(0, at);
      auto sf_hp = CharacterStatGameInstance::GetHp();
      CharacterStatGameInstance::SetHp(sf_hp + at.num);
    }
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    SkillGameInstance::Skis()[u"0001001"].cd =
        Window::dt_now_ + static_cast<uint64_t>(LoadSkiInt(u"0001001", ski_lv,
                                                           u"cooltime", 120)) *
                              1000;

    auto& ski = SkillGameInstance::ski_;
    g_skill.end();
    g_skill.lv = ski_lv;
    heal_amount = LoadSkiInt(u"0001001", ski_lv, u"x", 0);
    now = 0;
    g_skill.duration =
        static_cast<uint64_t>(LoadSkiInt(u"0001001", ski_lv, u"time", 30)) *
        1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    ski.push_back(g_skill);

    auto& sf = CharacterGameInstance::self_;
    auto ckt = SkillGameInstance::CreateSkillPayload(1001, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };

  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}

static void NimbleFeet() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"0001002";
  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    std::erase_if(ski, [](const GameSkill& s) { return s.id == u"0001002"; });
    // 速度由 buff 层重算后自动收回
    CharacterStatGameInstance::Update();
  };

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    SkillGameInstance::Skis()[u"0001002"].cd =
        Window::dt_now_ + static_cast<uint64_t>(LoadSkiInt(u"0001002", ski_lv,
                                                           u"cooltime", 180)) *
                              1000;

    auto& ski = SkillGameInstance::ski_;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration =
        static_cast<uint64_t>(LoadSkiInt(u"0001002", ski_lv, u"time", 30)) *
        1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    ski.push_back(g_skill);
    // WZ 的 speed 命中 buff 层, 由 CharacterStatGameInstance::UpdateMoveLimits
    // 生效
    CharacterStatGameInstance::Update();

    auto& sf = CharacterGameInstance::self_;
    auto ckt = SkillGameInstance::CreateSkillPayload(1002, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };

  auto& skills = SkillGameInstance::Skis();
  skills[g_skill.id] = g_skill;
}
[[maybe_unused]] static const bool r = [] {
  ThreeSnails();
  Recovery();
  NimbleFeet();
  return true;
}();

}  // namespace sdlms
