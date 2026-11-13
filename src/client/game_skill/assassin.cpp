#include "src/client/game/game_character.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_skill/assassin_attack.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <random>
#include <string>

namespace sdlms {

namespace {

int SkillInt(const char16_t* id, int lv, const char16_t* key, int32_t def) {
  return SkillGameInstance::LoadSkiLevelInt(
      std::u16string(id), static_cast<uint8_t>(std::max(1, lv)), key, def);
}

bool IsClaw(const GameCharacter& c) {
  return EquipGameInstance::LoadWeaponType(c) ==
         EquipGameInstance::WeaponType::kClaw;
}

GameTriangle AttackTriangle(int range) {
  return {{
      SDL_FPoint{static_cast<float>(-range), -100},
      SDL_FPoint{static_cast<float>(-range), 100},
      SDL_FPoint{0, -28},
  }};
}

void SendBuffState(int ski_id, int sub_val) {
  fbs::StateT st;
  st.state = fbs::StateEnum_BUFF_SKILL;
  st.val = ski_id;
  st.sub_val = sub_val;
  CharacterLogicSystem::ccs_.payload.push_back(
      std::make_unique<fbs::StateT>(st));
}

void SendSkill(uint32_t id, int lv, const fbs::ClientCharacterAttackT& cat) {
  auto ckt = SkillGameInstance::CreateSkillPayload(cat, id, lv);
  ServerCharacterInstance::HandleSki(ckt.ski_id, lv, ckt.payload, 0);
  ClientRequest::SendToHost(ckt);
}

// 4100000 Claw Mastery
static void ClawMastery() {
  GameSkill g_skill;
  g_skill.id = u"4100000";
  g_skill.passive = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    if (!IsClaw(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    CharacterStatGameInstance::SetSkiPro(
        SkillInt(u"4100000", lv, u"mastery", 0));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4100001 Critical Throw
static void CriticalThrow() {
  GameSkill g_skill;
  g_skill.id = u"4100001";
  g_skill.passive = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    if (!IsClaw(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 30);
    CharacterStatGameInstance::ApplyPassiveCritRate(static_cast<uint32_t>(
        std::max(0, SkillInt(u"4100001", lv, u"crtX", 0))));
    CharacterStatGameInstance::ApplyPassiveCritDamage(static_cast<uint32_t>(
        std::max(0, SkillInt(u"4100001", lv, u"crdX", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4100002 Critical Recovery
static void CriticalRecoveryPassive() {
  GameSkill g_skill;
  g_skill.id = u"4100002";
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4101000 Claw Booster
int claw_booster_base_speed = 0;
bool claw_booster_active = false;

void EndClawBooster() {
  if (!claw_booster_active) {
    return;
  }
  claw_booster_active = false;
  CharacterGameInstance::self_.attack_speed = claw_booster_base_speed;
}

static void ClawBooster() {
  GameSkill g_skill;
  g_skill.id = u"4101000";
  g_skill.can_use = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    return IsClaw(sf) &&
           JobSkillGameInstance::LoadSkiLevel(u"4100000") >= 5 &&
           CharacterStatGameInstance::GetHp() >
               SkillInt(u"4101000", ski_lv, u"hpCon", 0);
  };
  g_skill.use = [g_skill](uint64_t, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (!IsClaw(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    const int hp_con = SkillInt(u"4101000", lv, u"hpCon", 0);
    if (CharacterStatGameInstance::GetHp() <= hp_con) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"alert2");
    EndClawBooster();
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"4101000"; });
    CharacterStatGameInstance::SetHp(
        CharacterStatGameInstance::GetHp() - hp_con);
    claw_booster_base_speed = sf.attack_speed;
    claw_booster_active = true;
    sf.attack_speed += SkillInt(u"4101000", lv, u"x", -2);

    const int time_sec =
        std::max(1, SkillInt(u"4101000", lv, u"time", 1));
    g_skill.lv = static_cast<uint8_t>(lv);
    g_skill.duration = static_cast<uint64_t>(time_sec) * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    g_skill.end = []() {
      std::erase_if(SkillGameInstance::ski_,
                    [](const GameSkill& s) { return s.id == u"4101000"; });
      EndClawBooster();
      SendBuffState(4101000, 0);
      CharacterStatGameInstance::Update();
    };
    SkillGameInstance::ski_.push_back(g_skill);
    SendBuffState(4101000, 1);
    SendSkill(4101000, lv, fbs::ClientCharacterAttackT{});
    SkillGameInstance::Skis()[u"4101000"].cd = Window::dt_now_ + 1000;
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4101001 Haste
static void Haste() {
  GameSkill g_skill;
  g_skill.id = u"4101001";
  g_skill.climb = true;
  g_skill.can_use = [](int) {
    return IsClaw(CharacterGameInstance::self_);
  };
  g_skill.buff_stat = [](int lv, SkillBuffStat& stat) {
    stat.speed = std::max(0, SkillInt(u"4101001", lv, u"indieSpeed", 0));
    stat.jump = std::max(0, SkillInt(u"4101001", lv, u"indieJump", 0));
  };
  g_skill.end = []() {
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"4101001"; });
    CharacterStatGameInstance::Update();
    SendBuffState(4101001, 0);
  };
  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (!IsClaw(sf)) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"alert2");
    g_skill.end();
    g_skill.lv = static_cast<uint8_t>(std::clamp(ski_lv, 1, 20));
    g_skill.duration =
        static_cast<uint64_t>(std::max(
            1, SkillInt(u"4101001", g_skill.lv, u"time", 100))) *
        1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    SkillGameInstance::ski_.push_back(g_skill);
    CharacterStatGameInstance::Update();
    SendBuffState(4101001, 1);
    if (client_id != 0) {
      return;
    }
    auto r = SkillGameInstance::LoadSkiR(u"4101001");
    auto players = CharacterLogicSystem::RunBuffCheck(sf, r);
    auto ckt =
        SkillGameInstance::CreateSkillPayload(players, 4101001, g_skill.lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, g_skill.lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
    SkillGameInstance::Skis()[u"4101001"].cd = Window::dt_now_ + 1000;
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4101002 Drain
static void Drain() {
  GameSkill g_skill;
  g_skill.id = u"4101002";
  g_skill.can_use = [](int) {
    return IsClaw(CharacterGameInstance::self_) &&
           PackageGameInstance::LoadActiveBall() != nullptr;
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsClaw(sf)) {
      return;
    }
    auto ball = PackageGameInstance::LoadActiveBall();
    if (ball == nullptr) {
      return;
    }
    CharacterLogicSystem::RunAttackAction(sf, true);

    const int damage =
        std::max(0, SkillInt(u"4101002", ski_lv, u"damage", 180));
    const int absorb_rate =
        std::clamp(SkillInt(u"4101002", ski_lv, u"x", 5), 0, 100);
    auto cm = CharacterLogicSystem::RunAttackCheck(
        sf, AttackTriangle(350), 1);
    for (auto& mob : cm.data) {
      mob.hits = {damage};
    }

    const auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto pos = sf.pos;
    pos.y -= 28;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? 350 : -350;
    auto cct = BallGameInstance::CreateBallPayload(
        cm, pos, goal, delay, sf.page, 700,
        u"410.img/skill/4101002/effect");
    ClientRequest::SendToHost(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      const auto impact = BallGameInstance::LoadBallTime(cct);
      cat = SkillGameInstance::CreateAttackPayload(
          cm, sf.pos, impact, 0, "410.img/skill/4101002/effect/0", "");
      ClientRequest::SendToHost(cat);

      const int rolled =
          static_cast<int>(CharacterStatGameInstance::RollAttackDamage(
              static_cast<uint32_t>(damage)));
      const int max_hp =
          std::max(1, CharacterStatGameInstance::GetHpMax());
      const int heal = std::clamp(
          rolled * absorb_rate / 100, 0,
          std::min(max_hp / 4,
                   static_cast<int>(cm.data[0].mob.max_hp)));
      if (heal > 0) {
        CharacterStatGameInstance::SetHp(
            std::min(max_hp, CharacterStatGameInstance::GetHp() + heal));
      }
    }
    ItemGameInstance::DecItemNum(*ball, 1);
    SendSkill(4101002, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}


}  // namespace

void ApplyCriticalRecovery() {
  const auto& sf = CharacterGameInstance::self_;
  if (sf.job != u"410") {
    return;
  }
  const int lv = JobSkillGameInstance::LoadSkiLevel(u"4100002");
  if (lv <= 0) {
    return;
  }
  static uint64_t next_recovery = 0;
  if (Window::dt_now_ < next_recovery) {
    return;
  }
  const int prop =
      std::clamp(SkillInt(u"4100002", lv, u"prop", 0), 0, 100);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) >= prop) {
    return;
  }
  const int cool_sec =
      std::max(1, SkillInt(u"4100002", lv, u"cooltime", 30));
  next_recovery = Window::dt_now_ + static_cast<uint64_t>(cool_sec) * 1000;
  const int hp_rate =
      std::max(0, SkillInt(u"4100002", lv, u"x", 1));
  const int mp_rate =
      std::max(0, SkillInt(u"4100002", lv, u"y", 1));
  const int hp_max = std::max(1, CharacterStatGameInstance::GetHpMax());
  const int mp_max = std::max(1, CharacterStatGameInstance::GetMpMax());
  if (hp_rate > 0) {
    CharacterStatGameInstance::SetHp(std::min(
        hp_max, CharacterStatGameInstance::GetHp() + hp_max * hp_rate / 100));
  }
  if (mp_rate > 0) {
    CharacterStatGameInstance::SetMp(std::min(
        mp_max, CharacterStatGameInstance::GetMp() + mp_max * mp_rate / 100));
  }
}


void TryClawStarRecovery() {
  const auto& sf = CharacterGameInstance::self_;
  if (sf.job != u"410") {
    return;
  }
  const int lv = JobSkillGameInstance::LoadSkiLevel(u"4100000");
  if (lv <= 0) {
    return;
  }
  auto ball = PackageGameInstance::LoadActiveBall();
  if (ball == nullptr) {
    return;
  }
  const int prop =
      std::clamp(SkillInt(u"4100000", lv, u"prop", 0), 0, 100);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) < prop) {
    ItemGameInstance::AddItemNum(*ball, 1);
  }
}

[[maybe_unused]] static const bool r = [] {
  ClawMastery();
  CriticalThrow();
  CriticalRecoveryPassive();
  ClawBooster();
  Haste();
  Drain();
  return true;
}();

}  // namespace sdlms