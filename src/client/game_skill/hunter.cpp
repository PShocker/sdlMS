#include "src/client/game/game_character.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_skill/hunter_attack.h"
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
#include <any>
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

bool IsBow(const GameCharacter& c) {
  return EquipGameInstance::LoadWeaponType(c) ==
         EquipGameInstance::WeaponType::kBow;
}

// 与 BallGameInstance::ClosestPointOnRect 相同的判定：取矩形上离人物最近的点。
SDL_FPoint ClosestPointOnRect(const SDL_FPoint& pos, const SDL_FRect& rect) {
  SDL_FPoint closest;
  closest.x = std::clamp(pos.x, rect.x, rect.x + rect.w);
  closest.y = std::clamp(pos.y, rect.y, rect.y + rect.h);
  return closest;
}

void SendBuffState(int ski_id, int sub_val) {  fbs::StateT st;
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

void ApplyMobDebuff(GameDebuff::DebuffType type, CheckMobs& cm,
                    uint64_t delay, uint64_t duration, int64_t sub_val = 0) {
  fbs::ClientMobDebuffT cmb;
  cmb.map_id = SceneSystemInstance::map_id_;
  for (auto& mob : cm.data) {
    auto it = MobGameInstance::data_.find(mob.mob.index);
    if (it == MobGameInstance::data_.end() ||
        it->second.mob.debuffs.contains(type)) {
      continue;
    }
    auto md = std::make_unique<fbs::MobDebuffT>();
    md->mob_index = mob.mob.index;
    md->payload = std::make_unique<fbs::StateT>();
    md->payload->val = static_cast<int64_t>(type);
    md->payload->sub_val = sub_val;
    md->payload->delay = delay;
    md->payload->time = delay + duration;
    cmb.payload.push_back(std::move(md));
  }
  if (!cmb.payload.empty()) {
    ServerMobInstance::HandleDebuff(0, cmb);
  }
}

// 3100000 Bow Mastery
void BowMastery() {
  GameSkill g_skill;
  g_skill.id = u"3100000";
  g_skill.passive = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    if (!IsBow(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    CharacterStatGameInstance::SetSkiPro(SkillInt(u"3100000", lv, u"mastery", 0));
    CharacterStatGameInstance::SetSkiAvd(static_cast<uint32_t>(
        std::max(0, SkillInt(u"3100000", lv, u"x", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3100001 Amazon's Judgement
void AmazonJudgement() {
  GameSkill g_skill;
  g_skill.id = u"3100001";
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3101000 Final Attack: Bow（被动）
void FinalAttackBow() {
  GameSkill g_skill;
  g_skill.id = u"3101000";
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3101001 Bow Booster
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
  g_skill.id = u"3101001";
  g_skill.can_use = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    return IsBow(sf) &&
           JobSkillGameInstance::LoadSkiLevel(u"3100000") >= 5 &&
           CharacterStatGameInstance::GetHp() >
               SkillInt(u"3101001", ski_lv, u"hpCon", 0);
  };
  g_skill.use = [g_skill](uint64_t, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (!IsBow(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    const int hp_con = SkillInt(u"3101001", lv, u"hpCon", 0);
    if (CharacterStatGameInstance::GetHp() <= hp_con) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"alert2");
    EndBooster();
    auto& ski = SkillGameInstance::ski_;
    std::erase_if(ski,
                  [](const GameSkill& s) { return s.id == u"3101001"; });
    CharacterStatGameInstance::SetHp(
        CharacterStatGameInstance::GetHp() - hp_con);
    booster_base_speed = sf.attack_speed;
    booster_active = true;
    sf.attack_speed += SkillInt(u"3101001", lv, u"x", -2);

    const int time_sec =
        std::max(1, SkillInt(u"3101001", lv, u"time", 1));
    g_skill.lv = static_cast<uint8_t>(lv);
    g_skill.duration = static_cast<uint64_t>(time_sec) * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    g_skill.end = []() {
      std::erase_if(SkillGameInstance::ski_,
                    [](const GameSkill& s) { return s.id == u"3101001"; });
      EndBooster();
      SendBuffState(3101001, 0);
      CharacterStatGameInstance::Update();
    };
    ski.push_back(g_skill);
    SendBuffState(3101001, 1);
    SendSkill(3101001, lv, fbs::ClientCharacterAttackT{});
    SkillGameInstance::Skis()[u"3101001"].cd = Window::dt_now_ + 1000;
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3101003 Soul Arrow: Bow
void SoulArrowBow() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"3101003";
  g_skill.can_use = [](int) {
    return IsBow(CharacterGameInstance::self_) &&
           JobSkillGameInstance::LoadSkiLevel(u"3101001") >= 5;
  };
  g_skill.end = []() {
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"3101003"; });
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
  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (!IsBow(sf)) {
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
            1, SkillInt(u"3101003", g_skill.lv, u"time", 60))) *
        1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    SkillGameInstance::ski_.push_back(g_skill);
    SkillGameInstance::Skis()[u"3101003"].cd = Window::dt_now_ + 1000;
    SendBuffState(3101003, 1);
    SendSkill(3101003, g_skill.lv, fbs::ClientCharacterAttackT{});
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 3101004 Arrow Bomb: Bow
void ArrowBombBow() {
  GameSkill g_skill;
  g_skill.id = u"3101004";
  g_skill.can_use = [](int) {
    return IsBow(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsBow(sf)) {
      return;
    }
    CharacterLogicSystem::RunAttackAction(sf, true);

    GameTriangle tri = {{
        SDL_FPoint{-400, -100},
        SDL_FPoint{-400, 100},
        SDL_FPoint{0, -30},
    }};
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri, 1);
    const int direct_damage =
        std::max(0, SkillInt(u"3101004", ski_lv, u"damage", 0));
    for (auto& mob : cm.data) {
      mob.hits = {direct_damage};
    }

    const auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto pos = sf.pos;
    pos.y -= 30;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? 400 : -400;
    auto cct = BallGameInstance::CreateBallPayload(
        cm, pos, goal, delay, sf.page, 700, u"310.img/skill/3101004/ball");
    ClientRequest::SendToHost(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    const bool hit = !cm.data.empty();
    const auto impact = hit ? BallGameInstance::LoadBallTime(cct) : delay;
    if (hit) {
      // 3101005: Arrow Bomb explosion, up to 4 targets, 2s stun chance.
      // 直接命中的箭不再单独结算伤害，爆炸范围只结算 1 段。
      const auto mob_pos = cm.data[0].mob.pos;
      const int explosion_damage =
          std::max(0, SkillInt(u"3101005", ski_lv, u"damage", 0));
      const int mob_count =
          std::max(1, SkillInt(u"3101005", ski_lv, u"mobCount", 4));
      const int stun_prop =
          std::clamp(SkillInt(u"3101005", ski_lv, u"prop", 0), 0, 100);
      const int stun_ms =
          std::max(0, SkillInt(u"3101005", ski_lv, u"time", 0)) * 1000;
      auto* lv_node =
          SkillGameInstance::LoadSkiLevelNode(u"3101005",
                                             static_cast<uint8_t>(
                                                 std::clamp(ski_lv, 1, 30)));
      SDL_FPoint lt = {-130.0f, -100.0f};
      SDL_FPoint rb = {130.0f, 100.0f};
      if (lv_node != nullptr) {
        if (auto* n = lv_node->get_child(u"lt"); n != nullptr) {
          lt = WzResource::LoadFpoint(n);
        }
        if (auto* n = lv_node->get_child(u"rb"); n != nullptr) {
          rb = WzResource::LoadFpoint(n);
        }
      }
      auto rect = SkillGameInstance::LoadR(lt, rb, mob_pos, sf.flip);
      auto area = CharacterLogicSystem::RunAttackCheck(mob_pos, rect, mob_count);
      for (auto& mob : area.data) {
        mob.hits = {explosion_damage};
      }
      if (!area.data.empty()) {
        cat = SkillGameInstance::CreateAttackPayload(
            area, mob_pos, impact, 0, "310.img/skill/3101005/hit/0", "");
        ClientRequest::SendToHost(cat);
        if (stun_ms > 0) {
          std::uniform_int_distribution<int> roll(0, 99);
          if (roll(RandomGameInstance::gen_) < stun_prop) {
            ApplyMobDebuff(GameDebuff::DebuffType::kDizz, area, impact,
                           static_cast<uint64_t>(stun_ms));
          }
        }
      }
      // Arrow Bomb 是飞行道具攻击，命中后才检查 Ultimate Bow。
      TriggerBowFinalAttack(cm, impact);
    }
    SendSkill(3101004, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

}  // namespace

void ApplyAmazonJudgement(GameMob& mob, uint64_t delay) {
  const auto& sf = CharacterGameInstance::self_;
  if (sf.job != u"310") {
    return;
  }
  const int lv = JobSkillGameInstance::LoadSkiLevel(u"3100001");
  if (lv <= 0) {
    return;
  }
  const int prop = std::clamp(SkillInt(u"3100001", lv, u"prop", 0), 0, 100);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) >= prop) {
    return;
  }
  auto it = MobGameInstance::data_.find(mob.index);
  if (it == MobGameInstance::data_.end() ||
      it->second.mob.debuffs.contains(GameDebuff::DebuffType::kSlow)) {
    return;
  }
  const int speed = std::max(0, SkillInt(u"3100001", lv, u"speed", 0));
  const int time_ms =
      std::max(0, SkillInt(u"3100001", lv, u"time", 0)) * 1000;
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

void TriggerBowFinalAttack(CheckMobs& cm, uint64_t delay) {
  static bool in_progress = false;
  if (in_progress || cm.data.empty()) {
    return;
  }
  auto& sf = CharacterGameInstance::self_;
  if (sf.job != u"310" || !IsBow(sf)) {
    return;
  }
  const int lv = JobSkillGameInstance::LoadSkiLevel(u"3101000");
  if (lv <= 0 || JobSkillGameInstance::LoadSkiLevel(u"3100000") < 3) {
    return;
  }
  const int prop = std::clamp(SkillInt(u"3101000", lv, u"prop", 0), 0, 100);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) >= prop) {
    return;
  }

  in_progress = true;
  const int mob_count =
      std::max(1, SkillInt(u"3101006", lv, u"mobCount", 3));
  const int damage =
      std::max(0, SkillInt(u"3101006", lv, u"damage", 0));
  GameTriangle tri = {{
      SDL_FPoint{-400, -100},
      SDL_FPoint{-400, 100},
      SDL_FPoint{0, -30},
  }};
  auto fa_cm = CharacterLogicSystem::RunAttackCheck(sf, tri, mob_count);
  for (auto& mob : fa_cm.data) {
    mob.hits = {damage};
    // X 取离人物最近的点，Y 固定为怪物碰撞矩形中心。
    if (auto rect = MobLogicSystem::LoadRect(mob.mob); rect.has_value()) {
      auto hit_pos = ClosestPointOnRect(sf.pos, rect.value());
      mob.x = hit_pos.x - mob.mob.pos.x;
      mob.y = (rect->y + rect->h * 0.5f) - mob.mob.pos.y;
    }
  }
  fbs::ClientCharacterAttackT cat;
  if (!fa_cm.data.empty()) {
    // 与战士终极攻击一致：伤害包直接带 3101006/hit，不发送隐藏技能包。
    cat = SkillGameInstance::CreateAttackPayload(
        fa_cm, sf.pos, delay + 150, 0, "310.img/skill/3101006/hit/0", "");
    ClientRequest::SendToHost(cat);
  }
  in_progress = false;
}

[[maybe_unused]] static const bool r = [] {
  BowMastery();
  AmazonJudgement();
  FinalAttackBow();
  BowBooster();
  SoulArrowBow();
  ArrowBombBow();
  return true;
}();

}  // namespace sdlms
