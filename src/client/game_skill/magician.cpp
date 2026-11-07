
#include "src/client/game/game_ball.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
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

#include <algorithm>
#include <any>
#include <array>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace sdlms {

namespace {

int SkillInt(const char16_t* id, int lv, const char16_t* key, int32_t def) {
  return SkillGameInstance::LoadSkiLevelInt(
      std::u16string(id), static_cast<uint8_t>(std::max(1, lv)), key, def);
}

int SkillCommonInt(const char16_t* id, const char16_t* key, int32_t def) {
  auto* node = SkillGameInstance::LoadSkiNode(std::u16string(id));
  if (node == nullptr) {
    return def;
  }
  auto* common = node->get_child(u"common");
  if (common == nullptr) {
    return def;
  }
  auto* child = common->get_child(key);
  if (child == nullptr) {
    return def;
  }
  if (child->type == wz::Type::Int) {
    return static_cast<wz::Property<int32_t>*>(child)->get();
  }
  if (child->type == wz::Type::Long) {
    return static_cast<int32_t>(std::clamp<int64_t>(
        static_cast<wz::Property<int64_t>*>(child)->get(), INT32_MIN,
        INT32_MAX));
  }
  if (child->type == wz::Type::String) {
    auto text = static_cast<wz::Property<std::u16string>*>(child)->get();
    try {
      return std::stoi(std::string{text.begin(), text.end()});
    } catch (...) {
      return def;
    }
  }
  return def;
}

bool IsMagicWeapon(const GameCharacter& c) {
  auto weapon = EquipGameInstance::LoadWeaponType(c);
  // 短杖和长杖的 attack 类型都是 6。
  return weapon == EquipGameInstance::WeaponType::kWand;
}

void SendBuffState(int ski_id, int sub_val) {
  fbs::StateT st;
  st.state = fbs::StateEnum_BUFF_SKILL;
  st.val = ski_id;
  st.sub_val = sub_val;
  CharacterLogicSystem::ccs_.payload.push_back(
      std::make_unique<fbs::StateT>(st));
}

bool IsBuffActive(const char16_t* id) {
  const std::u16string skill_id(id);
  auto& ski = SkillGameInstance::ski_;
  return std::ranges::any_of(
      ski, [&](const GameSkill& s) { return s.id == skill_id; });
}

void SendSkill(uint32_t id, int lv, const fbs::ClientCharacterAttackT& cat) {
  auto ckt = SkillGameInstance::CreateSkillPayload(cat, id, lv);
  ServerCharacterInstance::HandleSki(ckt.ski_id, lv, ckt.payload, 0);
  ClientRequest::SendToHost(ckt);
}

}  // namespace

// 2001002 Energy Bolt：消耗 MP 攻击单个敌人，WZ mad 为技能伤害百分比。
static void EnergyBolt() {
  GameSkill g_skill;
  g_skill.id = u"2001002";
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsMagicWeapon(sf)) {
      return;
    }
    const std::array actions = {u"swingO1", u"swingO2", u"swingO3"};
    std::uniform_int_distribution<> dis(0, actions.size() - 1);
    CharacterLogicSystem::RunAction(
        sf, actions[dis(RandomGameInstance::gen_)]);

    const int range = std::max(1, SkillCommonInt(u"2001002", u"range", 350));
    const int mob_count = std::max(1, SkillInt(u"2001002", ski_lv, u"mobCount", 1));
    const int attack_count =
        std::max(1, SkillInt(u"2001002", ski_lv, u"attackCount", 1));
    const int32_t mad =
        std::max<int32_t>(0, SkillInt(u"2001002", ski_lv, u"mad", 0));
    const int mastery = std::clamp(
        SkillInt(u"2001002", ski_lv, u"mastery", 1), 1, 10);

    GameTriangle tri = {{
        SDL_FPoint{static_cast<float>(-range), -100},
        SDL_FPoint{static_cast<float>(-range), 100},
        SDL_FPoint{0, -30},
    }};
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri, mob_count);
    for (auto& mob : cm.data) {
      mob.hits.assign(static_cast<size_t>(attack_count), mad);
      mob.magic_mastery = mastery;
    }

    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto pos = sf.pos;
    pos.y -= 30;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? range : -range;

    const std::u16string path = u"200.img/skill/2001002/ball";
    auto cct = BallGameInstance::CreateBallPayload(
        cm, pos, goal, delay, sf.page, 700, path);
    // 没有命中时也要显示弹道。
    ClientRequest::SendToHost(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d);
      ClientRequest::SendToHost(cat);
    }
    SendSkill(2001002, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2001000 Magic Guard：按 WZ x% 把 HP 伤害转为 MP 伤害；
// 再次使用关闭，不重复扣 MP。
static void MagicGuard() {
  GameSkill g_skill;
  g_skill.id = u"2001000";
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_);
  };
  g_skill.skip_mp = []() { return IsBuffActive(u"2001000"); };

  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"2001000"; });
    if (it != ski.end()) {
      ski.erase(it);
      SendBuffState(2001000, 0);
    }
  };

  g_skill.hit = [](GameMob*, uint64_t n) -> uint64_t {
    const int lv = JobSkillGameInstance::LoadSkiLevel(u"2001000");
    if (lv <= 0) {
      return n;
    }
    const int percent = std::clamp(
        SkillInt(u"2001000", lv, u"x", 0), 0, 100);
    if (percent <= 0 || n == 0) {
      return n;
    }
    const uint64_t mp_damage = n * percent / 100;
    if (mp_damage == 0) {
      return n;
    }
    const auto mp = CharacterStatGameInstance::GetMp();
    // WZ 描述：MP 不足时全部按 HP 伤害处理。
    if (static_cast<uint64_t>(std::max(0, mp)) < mp_damage) {
      return n;
    }
    CharacterStatGameInstance::SetMp(
        static_cast<int32_t>(mp - static_cast<int64_t>(mp_damage)));
    return n - mp_damage;
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    // Magic Guard 开/关都进入 1 秒冷却。
    SkillGameInstance::Skis()[u"2001000"].cd = Window::dt_now_ + 1000;
    if (IsBuffActive(u"2001000")) {
      g_skill.end();
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"alert2");
    g_skill.lv = static_cast<uint8_t>(ski_lv);
    SkillGameInstance::ski_.push_back(g_skill);
    SendBuffState(2001000, 1);
    SendSkill(2001000, ski_lv, fbs::ClientCharacterAttackT{});
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2001001 Magic Armor：按 WZ indiePdd / indieMdd 增加物防魔防，time 秒持续。
static void MagicArmor() {
  GameSkill g_skill;
  g_skill.id = u"2001001";
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_);
  };

  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"2001001"; });
    if (it != ski.end()) {
      ski.erase(it);
      CharacterStatGameInstance::Update();
      SendBuffState(2001001, 0);
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
    if (!IsMagicWeapon(sf)) {
      return;
    }
    g_skill.end();

    const int time_sec = std::max(1, SkillInt(u"2001001", ski_lv, u"time", 300));
    g_skill.lv = static_cast<uint8_t>(ski_lv);
    g_skill.duration = static_cast<uint64_t>(time_sec) * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    SkillGameInstance::ski_.push_back(g_skill);

    CharacterLogicSystem::RunAction(sf, u"alert2");
    CharacterStatGameInstance::Update();
    SendBuffState(2001001, 1);
    SendSkill(2001001, ski_lv, fbs::ClientCharacterAttackT{});
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2001003 Magic Claw：WZ attackCount 次命中，mad 为每次伤害百分比。
static void MagicClaw() {
  GameSkill g_skill;
  g_skill.id = u"2001003";
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsMagicWeapon(sf)) {
      return;
    }
    const std::array actions = {u"swingO1", u"swingO2", u"swingO3"};
    std::uniform_int_distribution<> dis(0, actions.size() - 1);
    CharacterLogicSystem::RunAction(
        sf, actions[dis(RandomGameInstance::gen_)]);

    const int mob_count = std::max(1, SkillInt(u"2001003", ski_lv, u"mobCount", 1));
    const int attack_count =
        std::max(1, SkillInt(u"2001003", ski_lv, u"attackCount", 2));
    const int32_t mad =
        std::max<int32_t>(0, SkillInt(u"2001003", ski_lv, u"mad", 0));
    const int mastery = std::clamp(
        SkillInt(u"2001003", ski_lv, u"mastery", 1), 1, 10);

    auto g_r = SkillGameInstance::LoadSkiR(u"2001003");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r, mob_count);
    for (auto& mob : cm.data) {
      mob.hits.assign(static_cast<size_t>(attack_count), mad);
      mob.magic_mastery = mastery;
    }

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay, 120, "", "");
      ClientRequest::SendToHost(cat);
    }
    SendSkill(2001003, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2000000 Improved MP Recovery：每 10 秒恢复 x% Max MP，
// 并提高 y% 的 MP 药品恢复量。
static void ImprovedMpRecovery() {
  GameSkill g_skill;
  g_skill.id = u"2000000";
  g_skill.passive = [](int ski_lv) {
    const int lv = std::clamp(ski_lv, 1, 15);
    CharacterStatGameInstance::ApplyPassiveMpRecovery(
        static_cast<uint32_t>(std::max(0, SkillInt(u"2000000", lv, u"y", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2000001 Max MP Increase：WZ mmpR 提高 MaxMP 百分比。
static void MaxMpIncrease() {
  GameSkill g_skill;
  g_skill.id = u"2000001";
  g_skill.passive = [](int ski_lv) {
    const int lv = std::clamp(ski_lv, 1, 15);
    CharacterStatGameInstance::ApplyPassiveMpRate(
        std::max<int64_t>(0, SkillInt(u"2000001", lv, u"mmpR", 0)));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  ImprovedMpRecovery();
  MaxMpIncrease();
  MagicGuard();
  MagicArmor();
  EnergyBolt();
  MagicClaw();
  return true;
}();

}  // namespace sdlms
