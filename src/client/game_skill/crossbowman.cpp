#include "src/client/game/game_character.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_skill/crossbow_attack.h"
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
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_mob_instance.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace sdlms {

namespace {

int SkillInt(const std::u16string& id, int lv, const char16_t* key,
             int32_t def) {
  return SkillGameInstance::LoadSkiLevelInt(
      id, static_cast<uint8_t>(std::max(1, lv)), key, def);
}

bool IsCrossbow(const GameCharacter& c) {
  return EquipGameInstance::LoadWeaponType(c) ==
         EquipGameInstance::WeaponType::kCrossbow;
}

SDL_FPoint ClosestPointOnRect(const SDL_FPoint& pos, const SDL_FRect& rect) {
  return {std::clamp(pos.x, rect.x, rect.x + rect.w),
          std::clamp(pos.y, rect.y, rect.y + rect.h)};
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

GameTriangle AttackTriangle(int range) {
  return {{
      SDL_FPoint{static_cast<float>(-range), -100},
      SDL_FPoint{static_cast<float>(-range), 100},
      SDL_FPoint{0, -30},
  }};
}

// 3200000 Crossbow Mastery
void CrossbowMastery() {
  GameSkill g_skill;
  g_skill.id = u"3200000";
  g_skill.passive = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    if (!IsCrossbow(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    CharacterStatGameInstance::SetSkiPro(
        SkillInt(u"3200000", lv, u"mastery", 0));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3200001 Amazon's Judgement
void AmazonJudgement() {
  GameSkill g_skill;
  g_skill.id = u"3200001";
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3201000 Final Attack: Crossbow（被动）
void FinalAttackCrossbow() {
  GameSkill g_skill;
  g_skill.id = u"3201000";
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3201001 Crossbow Booster
int booster_base_speed = 0;
bool booster_active = false;

void EndBooster() {
  if (!booster_active) {
    return;
  }
  booster_active = false;
  CharacterGameInstance::self_.attack_speed = booster_base_speed;
}

void BowBooster() {
  GameSkill g_skill;
  g_skill.id = u"3201001";
  g_skill.can_use = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    return IsCrossbow(sf) &&
           JobSkillGameInstance::LoadSkiLevel(u"3200000") >= 5 &&
           CharacterStatGameInstance::GetHp() >
               SkillInt(u"3201001", ski_lv, u"hpCon", 0);
  };
  g_skill.use = [g_skill](uint64_t, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (!IsCrossbow(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    const int hp_con = SkillInt(u"3201001", lv, u"hpCon", 0);
    if (CharacterStatGameInstance::GetHp() <= hp_con) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"alert2");
    EndBooster();
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"3201001"; });
    CharacterStatGameInstance::SetHp(
        CharacterStatGameInstance::GetHp() - hp_con);
    booster_base_speed = sf.attack_speed;
    booster_active = true;
    sf.attack_speed += SkillInt(u"3201001", lv, u"x", -2);

    const int time_sec =
        std::max(1, SkillInt(u"3201001", lv, u"time", 1));
    g_skill.lv = static_cast<uint8_t>(lv);
    g_skill.duration = static_cast<uint64_t>(time_sec) * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    g_skill.end = []() {
      std::erase_if(SkillGameInstance::ski_,
                    [](const GameSkill& s) { return s.id == u"3201001"; });
      EndBooster();
      SendBuffState(3201001, 0);
      CharacterStatGameInstance::Update();
    };
    SkillGameInstance::ski_.push_back(g_skill);
    SendBuffState(3201001, 1);
    SendSkill(3201001, lv, fbs::ClientCharacterAttackT{});
    SkillGameInstance::Skis()[u"3201001"].cd = Window::dt_now_ + 1000;
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3201003 Soul Arrow: Crossbow
void SoulArrowCrossbow() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"3201003";
  g_skill.can_use = [](int) {
    return IsCrossbow(CharacterGameInstance::self_) &&
           JobSkillGameInstance::LoadSkiLevel(u"3201001") >= 5;
  };
  g_skill.end = []() {
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"3201003"; });
    CharacterStatGameInstance::Update();
  };
  g_skill.action = [](GameSkill* s, std::u16string ski_id) {
    auto b = PackageGameInstance::LoadActiveBall();
    if (b == nullptr) {
      return;
    }
    if (ski_id.empty()) {
      if (CharacterGameInstance::self_.action == u"shoot1") {
        ItemGameInstance::AddItemNum(*b, 1);
      }
    } else {
      auto lv = JobSkillGameInstance::LoadSkiLevel(ski_id);
      ItemGameInstance::AddItemNum(
          *b, SkillGameInstance::LoadSkiBallNum(ski_id, lv));
    }
  };
  g_skill.frame = []() { return; };
  g_skill.use = [g_skill](uint64_t, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (!IsCrossbow(sf)) {
      return;
    }
    if (CharacterLogicSystem::LoadActionType(sf) !=
        CharacterLogicSystem::ActionEnum::kClimb) {
      CharacterLogicSystem::RunAction(sf, u"alert2");
    } else {
      CharacterLogicSystem::self_climb_animate_cooldown_ =
          Window::dt_now_ + 700;
    }
    g_skill.end();
    g_skill.lv = static_cast<uint8_t>(std::clamp(ski_lv, 1, 20));
    g_skill.duration =
        static_cast<uint64_t>(std::max(
            1, SkillInt(u"3201003", g_skill.lv, u"time", 60))) *
        1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    SkillGameInstance::ski_.push_back(g_skill);
    SkillGameInstance::Skis()[u"3201003"].cd = Window::dt_now_ + 1000;
    SendBuffState(3201003, 1);
    SendSkill(3201003, g_skill.lv, fbs::ClientCharacterAttackT{});
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3201004 Iron Arrow: Crossbow，最多穿 4 个目标，伤害逐個衰减。
void IronArrowCrossbow() {
  GameSkill g_skill;
  g_skill.id = u"3201004";
  g_skill.can_use = [](int) {
    return IsCrossbow(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsCrossbow(sf)) {
      return;
    }
    CharacterLogicSystem::RunAttackAction(sf, true);

    const int mob_count =
        std::max(1, SkillInt(u"3201004", ski_lv, u"mobCount", 4));
    const auto cast_delay = SkillGameInstance::LoadSkiTime(sf);
    auto pos = sf.pos;
    pos.y -= 30;
    constexpr float kRange = 500.0f;
    constexpr float kHalfHeight = 10.0f;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? kRange : -kRange;

    // 095：Iron Arrow 使用横向攻击带，箭矢沿该带持续飞行。
    SDL_FRect band = {
        .x = std::min(pos.x, goal.x),
        .y = pos.y - kHalfHeight,
        .w = std::fabs(goal.x - pos.x),
        .h = kHalfHeight * 2.0f + 1.0f,
    };
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, band, mob_count);

    const int base_damage =
        std::max(0, SkillInt(u"3201004", ski_lv, u"damage", 0));
    for (size_t i = 0; i < cm.data.size(); ++i) {
      const int damage =
          std::max(1, base_damage -
                          base_damage * static_cast<int>(i) * 20 / 100);
      cm.data[i].hits = {damage};
    }

    CheckMobs visual_cm;
    auto cct = BallGameInstance::CreateBallPayload(
        visual_cm, pos, goal, cast_delay, sf.page, 800,
        u"320.img/skill/3201004/ball");
    ClientRequest::SendToHost(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      for (auto& mob : cm.data) {
        const float dx = std::fabs(mob.mob.pos.x - pos.x);
        mob.delay = static_cast<uint64_t>(dx * 1000.0f / 800.0f);
        // 095 CMob::GetHitPoint：攻击带与怪物本体矩形交集的中点。
        if (auto rect = MobLogicSystem::LoadRect(mob.mob); rect.has_value()) {
          SDL_FRect hit{};
          if (SDL_GetRectIntersectionFloat(&rect.value(), &band, &hit)) {
            mob.x = (hit.x + hit.w * 0.5f) - mob.mob.pos.x;
            mob.y = (hit.y + hit.h * 0.5f) - mob.mob.pos.y;
          }
        }
      }
      cat = SkillGameInstance::CreateAttackPayload(
          cm, sf.pos, cast_delay, 0, "320.img/skill/3201004/hit/0", "");
      ClientRequest::SendToHost(cat);
      TriggerCrossbowFinalAttack(cm, cast_delay);
    }
    SendSkill(3201004, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

}  // namespace

void ApplyCrossbowAmazonJudgement(GameMob& mob, uint64_t delay) {
  const auto& sf = CharacterGameInstance::self_;
  if (sf.job != u"320" || !IsCrossbow(sf)) {
    return;
  }
  const int lv = JobSkillGameInstance::LoadSkiLevel(u"3200001");
  if (lv <= 0 || JobSkillGameInstance::LoadSkiLevel(u"3200000") < 5) {
    return;
  }
  const int prop =
      std::clamp(SkillInt(u"3200001", lv, u"prop", 0), 0, 100);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) >= prop) {
    return;
  }
  auto it = MobGameInstance::data_.find(mob.index);
  if (it == MobGameInstance::data_.end() ||
      it->second.mob.debuffs.contains(GameDebuff::DebuffType::kSlow)) {
    return;
  }
  const int speed = std::max(0, SkillInt(u"3200001", lv, u"speed", 0));
  const int time_ms =
      std::max(0, SkillInt(u"3200001", lv, u"time", 0)) * 1000;
  fbs::ClientMobDebuffT cmb;
  cmb.map_id = SceneSystemInstance::map_id_;
  auto md = std::make_unique<fbs::MobDebuffT>();
  md->mob_index = mob.index;
  md->payload = std::make_unique<fbs::StateT>();
  md->payload->val = static_cast<int64_t>(GameDebuff::DebuffType::kSlow);
  md->payload->sub_val = speed;
  md->payload->delay = delay;
  md->payload->time = delay + time_ms;
  cmb.payload.push_back(std::move(md));
  ServerMobInstance::HandleDebuff(0, cmb);
}

void TriggerCrossbowFinalAttack(CheckMobs& cm, uint64_t delay) {
  static bool in_progress = false;
  if (in_progress || cm.data.empty()) {
    return;
  }
  auto& sf = CharacterGameInstance::self_;
  if (sf.job != u"320" || !IsCrossbow(sf)) {
    return;
  }
  const int lv = JobSkillGameInstance::LoadSkiLevel(u"3201000");
  if (lv <= 0 || JobSkillGameInstance::LoadSkiLevel(u"3200000") < 3) {
    return;
  }
  const int prop =
      std::clamp(SkillInt(u"3201000", lv, u"prop", 0), 0, 100);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) >= prop) {
    return;
  }

  in_progress = true;
  const int damage =
      std::max(0, SkillInt(u"3201005", lv, u"damage", 0));
  auto fa_cm = CharacterLogicSystem::RunAttackCheck(sf, AttackTriangle(500), 1);
  for (auto& mob : fa_cm.data) {
    mob.hits = {damage};
    if (auto rect = MobLogicSystem::LoadRect(mob.mob); rect.has_value()) {
      auto hit_pos = ClosestPointOnRect(sf.pos, rect.value());
      mob.x = hit_pos.x - mob.mob.pos.x;
      mob.y = (rect->y + rect->h * 0.5f) - mob.mob.pos.y;
    }
  }
  if (!fa_cm.data.empty()) {
    auto cat = SkillGameInstance::CreateAttackPayload(
        fa_cm, sf.pos, delay + 150, 0, "320.img/skill/3201005/hit/0", "");
    ClientRequest::SendToHost(cat);
  }
  in_progress = false;
}

[[maybe_unused]] static const bool r = [] {
  CrossbowMastery();
  AmazonJudgement();
  FinalAttackCrossbow();
  BowBooster();
  SoulArrowCrossbow();
  IronArrowCrossbow();
  return true;
}();

}  // namespace sdlms
