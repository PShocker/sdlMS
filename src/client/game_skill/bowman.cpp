#include "src/client/game/game_character.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_mob_instance.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <random>
#include <string>

namespace sdlms {

namespace {

int32_t SkillInt(const char16_t* id, int lv, const char16_t* key, int32_t def) {
  return SkillGameInstance::LoadSkiLevelInt(
      std::u16string(id), static_cast<uint8_t>(std::max(1, lv)), key, def);
}

bool IsBowWeapon(const GameCharacter& c) {
  auto weapon = EquipGameInstance::LoadWeaponType(c);
  return weapon == EquipGameInstance::WeaponType::kBow ||
         weapon == EquipGameInstance::WeaponType::kCrossbow;
}

SDL_FRect AttackRect(const GameCharacter& sf, int range) {
  SDL_FPoint lt{static_cast<float>(-range), -100};
  SDL_FPoint rb{0, 0};
  return SkillGameInstance::LoadR(lt, rb, sf.pos, sf.flip);
}

int RangeWithEye(const char16_t* id, int lv, int fallback) {
  return SkillInt(id, lv, u"range", fallback) +
         static_cast<int>(CharacterStatGameInstance::GetAttackRangeBonus());
}

}  // namespace

// 3000000 Critical Shot：暴击率 crtX，暴击伤害 crdX。
static void CriticalShot() {
  GameSkill g_skill;
  g_skill.id = u"3000000";
  g_skill.passive = [](int ski_lv) {
    const int lv = std::clamp(ski_lv, 1, 15);
    CharacterStatGameInstance::ApplyPassiveCritRate(static_cast<uint32_t>(
        std::max(0, SkillInt(u"3000000", lv, u"crtX", 0))));
    CharacterStatGameInstance::ApplyPassiveCritDamage(static_cast<uint32_t>(
        std::max(0, SkillInt(u"3000000", lv, u"crdX", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3000001 The Eye of Amazon：增加射程 range。
static void EyeOfAmazon() {
  GameSkill g_skill;
  g_skill.id = u"3000001";
  g_skill.passive = [](int ski_lv) {
    const int lv = std::clamp(ski_lv, 1, 15);
    CharacterStatGameInstance::ApplyPassiveAttackRange(static_cast<uint32_t>(
        std::max(0, SkillInt(u"3000001", lv, u"range", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3001000 Focus：短时间提高命中与回避。
static void Focus() {
  GameSkill g_skill;
  g_skill.id = u"3001000";
  g_skill.climb = true;

  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"3001000"; });
    if (it != ski.end()) {
      ski.erase(it);
      CharacterStatGameInstance::Update();
      fbs::StateT st;
      st.state = fbs::StateEnum_BUFF_SKILL;
      st.val = 3001000;
      st.sub_val = 0;
      CharacterLogicSystem::ccs_.payload.push_back(
          std::make_unique<fbs::StateT>(st));
    }
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    auto& ski = SkillGameInstance::ski_;
    g_skill.end();

    const int lv = std::clamp(ski_lv, 1, 20);
    int duration_sec = SkillInt(u"3001000", lv, u"time", 0);
    if (duration_sec <= 0) {
      duration_sec = 70;
    }
    g_skill.lv = static_cast<uint8_t>(lv);
    g_skill.duration = static_cast<uint64_t>(duration_sec) * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    ski.push_back(g_skill);

    CharacterLogicSystem::RunAction(sf, u"alert2");
    CharacterStatGameInstance::Update();

    auto ckt = SkillGameInstance::CreateSkillPayload(3001000, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);

    fbs::StateT st;
    st.state = fbs::StateEnum_BUFF_SKILL;
    st.val = 3001000;
    st.sub_val = 1;
    CharacterLogicSystem::ccs_.payload.push_back(
        std::make_unique<fbs::StateT>(st));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3001001 Arrow Blow：单箭，160% -> 240%。
static void ArrowBlow() {
  GameSkill g_skill;
  g_skill.id = u"3001001";
  g_skill.can_use = [](int) {
    auto& sf = CharacterGameInstance::self_;
    return IsBowWeapon(sf) && PackageGameInstance::LoadActiveBall() != nullptr;
  };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsBowWeapon(sf)) {
      return;
    }
    auto ball = PackageGameInstance::LoadActiveBall();
    if (ball == nullptr) {
      return;
    }

    CharacterLogicSystem::RunAttackAction(sf, true);
    const int range = RangeWithEye(u"3001001", ski_lv, 350);
    auto cm =
        CharacterLogicSystem::RunAttackCheck(sf, AttackRect(sf, range), 1);

    const int32_t damage =
        std::max<int32_t>(0, SkillInt(u"3001001", ski_lv, u"damage", 0));
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto pos = sf.pos;
    pos.y -= 28;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? range : -range;

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      cm.data[0].y = 0;
      cm.data[0].hits = {damage};
      auto path = BallGameInstance::LoadBallPath((*ball)->id);
      auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay, page,
                                                     700, path);
      ClientRequest::SendToHost(cct);
      ServerBallInstance::HandleServerB(cct.payload);
      auto d = BallGameInstance::LoadBallTime(cct);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d);
      ClientRequest::SendToHost(cat);
    }
    ItemGameInstance::DecItemNum(*ball, 1);

    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 3001001, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3001002 Double Shot：连续两支箭，每支 80% -> 120%。
static void DoubleShot() {
  GameSkill g_skill;
  g_skill.id = u"3001002";
  g_skill.can_use = [](int) {
    auto& sf = CharacterGameInstance::self_;
    return IsBowWeapon(sf) && PackageGameInstance::LoadActiveBall() != nullptr;
  };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsBowWeapon(sf)) {
      return;
    }
    auto ball = PackageGameInstance::LoadActiveBall();
    if (ball == nullptr) {
      return;
    }

    CharacterLogicSystem::RunAttackAction(sf, true);
    const int range = RangeWithEye(u"3001002", ski_lv, 350);
    auto cm =
        CharacterLogicSystem::RunAttackCheck(sf, AttackRect(sf, range), 1);

    const int32_t damage =
        std::max<int32_t>(0, SkillInt(u"3001002", ski_lv, u"damage", 0));
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto pos = sf.pos;
    pos.y -= 32;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? range : -range;
    auto arrow_path = BallGameInstance::LoadBallPath((*ball)->id);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      cm.data[0].hits = {damage, damage};
      auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay, page,
                                                     750, arrow_path);
      if (cct.payload->ball->mob_index != UINT32_MAX &&
          cct.payload->ball->y2 >= -8) {
        cct.payload->ball->y2 -= 8;
      }
      auto d = BallGameInstance::LoadBallTime(cct);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d, 120, "", "");
      ClientRequest::SendToHost(cat);

      for (int i = 0; i < 2; ++i) {
        cct.payload->ball->y1 = sf.pos.y - 28;
        cct.payload->ball->y2 += i * 8;
        ClientRequest::SendToHost(cct);
        ServerBallInstance::HandleServerB(cct.payload);
      }
    }
    ItemGameInstance::DecItemNum(*ball, 2);

    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 3001002, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3001003 Power Knockback：把多个敌人向后击退，不造成眩晕。
static void PowerKnockback() {
  GameSkill g_skill;
  g_skill.id = u"3001003";
  g_skill.fall = true;
  g_skill.can_use = [](int) {
    return IsBowWeapon(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsBowWeapon(sf)) {
      return;
    }

    CharacterLogicSystem::RunAttackAction(sf, false);
    const int wz_range = SkillInt(u"3001003", ski_lv, u"range", 130);
    const int range =
        wz_range +
        static_cast<int>(CharacterStatGameInstance::GetAttackRangeBonus());
    const int mob_count =
        std::clamp(SkillInt(u"3001003", ski_lv, u"mobCount", 1), 1, 4);
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, AttackRect(sf, range),
                                                   mob_count);
    const int32_t damage =
        std::max<int32_t>(0, SkillInt(u"3001003", ski_lv, u"damage", 0));
    // WZ 没有独立的击退距离字段：范围 130/140/150 对应 140/170/200。
    const float knockback =
        static_cast<float>(140 + (std::clamp(wz_range, 130, 150) - 130) * 3);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      for (auto& mob : cm.data) {
        mob.hits = {damage};
      }
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay, 60, "",
                                                   "", knockback);
      ClientRequest::SendToHost(cat);
    }

    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 3001003, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  CriticalShot();
  EyeOfAmazon();
  Focus();
  ArrowBlow();
  DoubleShot();
  PowerKnockback();
  return true;
}();

}  // namespace sdlms
