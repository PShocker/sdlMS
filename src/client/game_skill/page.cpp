#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_foothold.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/game_skill/page_final_attack.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_mob_instance.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <string>

namespace sdlms {

namespace {

int SkillInt(const std::u16string& id, int lv, const char16_t* key, int fallback) {
  return SkillGameInstance::LoadSkiLevelInt(
      id, static_cast<uint8_t>(std::max(1, lv)), key, fallback);
}

std::u16string U16(uint32_t id) {
  auto s = std::to_string(id);
  return std::u16string{s.begin(), s.end()};
}

int U16ToInt(const std::u16string& id) {
  try {
    return std::stoi(std::string{id.begin(), id.end()});
  } catch (...) {
    return 0;
  }
}

bool IsSword(const GameCharacter& c) {
  if (!c.weapon.has_value()) {
    return false;
  }
  auto w = EquipGameInstance::LoadWeaponEnum(c.weapon->id);
  return w == EquipGameInstance::WeaponEnum::kSword1H ||
         w == EquipGameInstance::WeaponEnum::kSword2H;
}

bool IsBlunt(const GameCharacter& c) {
  if (!c.weapon.has_value()) {
    return false;
  }
  auto w = EquipGameInstance::LoadWeaponEnum(c.weapon->id);
  return w == EquipGameInstance::WeaponEnum::kMace1H ||
         w == EquipGameInstance::WeaponEnum::kMace2H;
}

bool IsPageWeapon(const GameCharacter& c) {
  return IsSword(c) || IsBlunt(c);
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

void EraseSkill(const std::u16string& id) {
  auto& ski = SkillGameInstance::ski_;
  auto it = std::ranges::find_if(
      ski, [&id](const GameSkill& s) { return s.id == id; });
  if (it != ski.end()) {
    ski.erase(it);
  }
}

bool IsActive(const std::u16string& id) {
  const auto& ski = SkillGameInstance::ski_;
  return std::ranges::any_of(
      ski, [&id](const GameSkill& s) { return s.id == id; });
}

void StopSkill(const std::u16string& id) {
  EraseSkill(id);
  CharacterStatGameInstance::Update();
  SendBuffState(U16ToInt(id), 0);
}

void ExtraHit(GameCharacter& sf, int percent, int mob_count) {
  auto rect = AfterimageGameInstance::LoadRect(sf);
  if (!rect.has_value() || percent <= 0) {
    return;
  }
  auto cm = CharacterLogicSystem::RunAttackCheck(sf, *rect, mob_count);
  if (cm.data.empty()) {
    return;
  }
  for (auto& mob : cm.data) {
    mob.hits = {percent};
  }
  auto delay = SkillGameInstance::LoadSkiTime(sf);
  auto cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay);
  ClientRequest::SendToHost(cat);
}

int booster_base_speed = 0;
bool booster_active = false;

void EndBooster() {
  if (!booster_active) {
    return;
  }
  booster_active = false;
  CharacterGameInstance::self_.attack_speed = booster_base_speed;
}

// 1200000 Sword Mastery / 1200001 Blunt Weapon Mastery
void Mastery(uint32_t id, bool sword) {
  GameSkill g_skill;
  g_skill.id = U16(id);
  g_skill.passive = [id, sword](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    if (sword ? !IsSword(sf) : !IsBlunt(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    CharacterStatGameInstance::SetSkiPro(SkillInt(U16(id), lv,
                                                  u"mastery", 0));
    if (sword) {
      CharacterStatGameInstance::SetSkiDef(
          SkillInt(U16(id), lv, u"x", 0));
    }
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 1201000/1201001 与 Fighter 一样由攻击自动触发，不再作为主动开关。
void FinalAttackPassive(uint32_t id) {
  GameSkill g_skill;
  g_skill.id = U16(id);
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 1201002/1201003 Booster：消耗 HP/MP，提升攻击速度。
void Booster(uint32_t id, bool sword) {
  GameSkill g_skill;
  g_skill.id = U16(id);
  g_skill.fall = true;
  g_skill.can_use = [sword, id](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    if (sword ? !IsSword(sf) : !IsBlunt(sf)) {
      return false;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    const int hp_con =
        SkillInt(U16(id), lv, u"hpCon", 0);
    return CharacterStatGameInstance::GetHp() > hp_con;
  };
  g_skill.use = [g_skill, sword, id](uint64_t, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (sword ? !IsSword(sf) : !IsBlunt(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    const int hp_con = SkillInt(U16(id), lv, u"hpCon", 0);
    if (CharacterStatGameInstance::GetHp() <= hp_con) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"alert2");
    EndBooster();
    StopSkill(U16(id));
    CharacterStatGameInstance::SetHp(
        CharacterStatGameInstance::GetHp() - hp_con);
    booster_base_speed = sf.attack_speed;
    booster_active = true;
    sf.attack_speed += SkillInt(U16(id), lv, u"x", -2);

    const int time_sec = std::max(
        1, SkillInt(U16(id), lv, u"time", 1));
    g_skill.lv = static_cast<uint8_t>(lv);
    g_skill.duration = static_cast<uint64_t>(time_sec) * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    g_skill.end = [id]() {
      EraseSkill(U16(id));
      EndBooster();
      SendBuffState(static_cast<int>(id), 0);
      CharacterStatGameInstance::Update();
    };
    SkillGameInstance::ski_.push_back(g_skill);
    SendBuffState(static_cast<int>(id), 1);
    SendSkill(id, lv, fbs::ClientCharacterAttackT{});
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 1201004 Threaten：范围 4 个怪物，降低移动速度和物防。
void Threaten() {
  GameSkill g_skill;
  g_skill.id = u"1201004";
  g_skill.fall = true;
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAction(sf, u"alert2");
    const int mob_count =
        std::max(1, SkillInt(u"1201004", ski_lv, u"mobCount", 4));
    const int speed =
        std::max(0, SkillInt(u"1201004", ski_lv, u"speed", 0));
    const int wdef = std::abs(SkillInt(u"1201004", ski_lv, u"x", 0));
    const int time_sec =
        std::max(1, SkillInt(u"1201004", ski_lv, u"time", 10));
    auto g_r = SkillGameInstance::LoadSkiR(u"1201004");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r, mob_count);
    const auto attack_delay = SkillGameInstance::LoadSkiTime(sf);

    fbs::ClientMobDebuffT cmb;
    cmb.map_id = SceneSystemInstance::map_id_;
    for (auto& mob : cm.data) {
      auto it = MobGameInstance::data_.find(mob.mob.index);
      if (it != MobGameInstance::data_.end() &&
          it->second.mob.debuffs.contains(GameDebuff::DebuffType::kThreaten)) {
        continue;
      }
      auto md = std::make_unique<fbs::MobDebuffT>();
      md->mob_index = mob.mob.index;
      md->payload = std::make_unique<fbs::StateT>();
      md->payload->val =
          static_cast<int64_t>(GameDebuff::DebuffType::kThreaten);
      md->payload->sub_val = static_cast<int64_t>(speed & 0xFFFF) |
                             (static_cast<int64_t>(wdef & 0xFFFF) << 16);
      md->payload->delay = attack_delay;
      md->payload->time =
          attack_delay + static_cast<uint64_t>(time_sec) * 1000;
      cmb.payload.push_back(std::move(md));
    }
    if (!cmb.payload.empty()) {
      ServerMobInstance::HandleDebuff(0, cmb);
    }
    SendSkill(1201004, ski_lv, fbs::ClientCharacterAttackT{});
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 1201005 Rush：与 Fighter Rush 相同，沿 foothold 推进。
void Rush() {
  GameSkill g_skill;
  g_skill.id = u"1201005";
  g_skill.fall = true;
  g_skill.can_use = [](int) {
    return IsPageWeapon(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsPageWeapon(sf)) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"rushO");
    const int mob_count =
        std::max(1, SkillInt(u"1201005", ski_lv, u"mobCount", 4));
    auto g_r = SkillGameInstance::LoadSkiR(u"1201005");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r, mob_count);
    const int damage =
        std::max(0, SkillInt(u"1201005", ski_lv, u"damage", 60));
    for (auto& mob : cm.data) {
      mob.hits = {damage};
    }
    fbs::ClientCharacterAttackT cat;
    const int range =
        std::max(0, SkillInt(u"1201005", ski_lv, u"range", 130));
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      auto hit_type = AfterimageGameInstance::LoadHitType(sf);
      std::string effect =
          "Afterimage/hit.img/" + std::string(hit_type.begin(), hit_type.end());
      cat = SkillGameInstance::CreateAttackPayload(
          cm, sf.pos, delay, 0, effect, "", static_cast<float>(range));
      for (auto& attack : cat.payload) {
        attack->attack->knockback_speed = 1.6f;
      }
      RegisterPageFinalAttack(sf, delay);
    }
    struct RushState {
      float remaining = 0.0f;
      float dir = 0.0f;
    };
    auto rush = std::make_shared<RushState>();
    rush->remaining = static_cast<float>(range);
    rush->dir = sf.flip ? 1.0f : -1.0f;
    CharacterLogicSystem::self_invincible_cooldown_ = std::max(
        CharacterLogicSystem::self_invincible_cooldown_, Window::dt_now_ + 100);

    GameSkill move;
    move.id = u"1201005";
    move.destroy = Window::dt_now_ + 1000;
    move.frame = [rush]() {
      if (rush->remaining > 0.0f) {
        CharacterLogicSystem::self_invincible_cooldown_ =
            std::max(CharacterLogicSystem::self_invincible_cooldown_,
                     Window::dt_now_ + 100);
      }
      auto& sf = CharacterGameInstance::self_;
      for (int guard = 0; rush->remaining > 0.0f && guard < 8; ++guard) {
        const int32_t fh_id = CharacterLogicSystem::self_fh_;
        auto fh_it = FootholdGameInstance::data_.find(fh_id);
        if (fh_id == 0 || fh_it == FootholdGameInstance::data_.end()) {
          rush->remaining = 0.0f;
          return;
        }
        const auto& fh = fh_it->second;
        const float min_x = static_cast<float>(std::min(fh.x1, fh.x2));
        const float max_x = static_cast<float>(std::max(fh.x1, fh.x2));
        const float endpoint = rush->dir > 0.0f ? max_x : min_x;
        const float step =
            std::min(rush->remaining, 700.0f * Window::delta_time_ / 1000.0f);
        const float old_x = sf.pos.x;
        float target_x = std::clamp(old_x + rush->dir * step, min_x, max_x);
        const float moved = std::fabs(target_x - old_x);
        if (moved > 0.0f) {
          sf.pos.x = target_x;
          if (fh.k.has_value() && fh.intercept.has_value()) {
            sf.pos.y = target_x * fh.k.value() + fh.intercept.value();
          } else if (fh.x1 != fh.x2) {
            const float t =
                (target_x - fh.x1) / static_cast<float>(fh.x2 - fh.x1);
            sf.pos.y = static_cast<float>(fh.y1) +
                       (static_cast<float>(fh.y2 - fh.y1) * t);
          }
          rush->remaining -= moved;
        }
        if (rush->remaining <= 0.0f) {
          return;
        }
        if (std::fabs(sf.pos.x - endpoint) > 0.01f) {
          return;
        }
        const int32_t next_id = rush->dir > 0.0f ? fh.next : fh.prev;
        if (next_id == 0 || next_id == fh_id) {
          rush->remaining = 0.0f;
          return;
        }
        int32_t best_id = 0;
        SDL_FPoint best_entry{};
        float best_gap = std::numeric_limits<float>::max();
        for (const int32_t candidate_id : {fh.next, fh.prev}) {
          if (candidate_id == 0 || candidate_id == fh_id) {
            continue;
          }
          auto next_it = FootholdGameInstance::data_.find(candidate_id);
          if (next_it == FootholdGameInstance::data_.end()) {
            continue;
          }
          const auto& next = next_it->second;
          const float next_min =
              static_cast<float>(std::min(next.x1, next.x2));
          const float next_max =
              static_cast<float>(std::max(next.x1, next.x2));
          if (rush->dir > 0.0f && next_max <= sf.pos.x + 1.0f) {
            continue;
          }
          if (rush->dir < 0.0f && next_min >= sf.pos.x - 1.0f) {
            continue;
          }
          const SDL_FPoint a{static_cast<float>(next.x1),
                             static_cast<float>(next.y1)};
          const SDL_FPoint b{static_cast<float>(next.x2),
                             static_cast<float>(next.y2)};
          const auto dist2 = [](SDL_FPoint p) {
            const float dx = CharacterGameInstance::self_.pos.x - p.x;
            const float dy = CharacterGameInstance::self_.pos.y - p.y;
            return dx * dx + dy * dy;
          };
          const bool use_a = dist2(a) <= dist2(b);
          const SDL_FPoint entry = use_a ? a : b;
          const float gap = std::sqrt(dist2(entry));
          if (gap < best_gap) {
            best_gap = gap;
            best_id = candidate_id;
            best_entry = entry;
          }
        }
        if (best_id == 0) {
          rush->remaining = 0.0f;
          return;
        }
        CharacterLogicSystem::self_fh_ = best_id;
        sf.pos = best_entry;
        rush->remaining -= best_gap;
      }
    };
    move.end = []() { EraseSkill(u"1201005"); };
    SkillGameInstance::ski_.push_back(move);

    if (!cat.payload.empty()) {
      ClientRequest::SendToHost(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 1201005, ski_lv);
    ckt.payload.clear();
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

}  // namespace

namespace {

struct PendingPageFinalAttack {
  bool active = false;
  uint64_t due = 0;
  uint8_t lv = 0;
  bool sword = false;
};

PendingPageFinalAttack pending_page_final_attack;

}  // namespace

void RegisterPageFinalAttack(GameCharacter& g_character,
                             uint64_t attack_delay) {
  if (g_character.job != u"120") {
    return;
  }
  const bool sword = IsSword(g_character);
  const bool blunt = IsBlunt(g_character);
  if (!sword && !blunt) {
    return;
  }
  const int mastery = JobSkillGameInstance::LoadSkiLevel(
      sword ? u"1200000" : u"1200001");
  if (mastery < 3) {
    return;
  }
  const uint32_t id = sword ? 1201000 : 1201001;
  const int lv = JobSkillGameInstance::LoadSkiLevel(U16(id));
  if (lv <= 0) {
    return;
  }
  const int prop = SkillInt(U16(id), lv, u"prop", 0);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) >= prop) {
    return;
  }
  pending_page_final_attack.active = true;
  pending_page_final_attack.due = attack_delay + 150;
  pending_page_final_attack.lv = static_cast<uint8_t>(lv);
  pending_page_final_attack.sword = sword;
}

void UpdatePageFinalAttack() {
  if (!pending_page_final_attack.active ||
      Window::dt_time_ < pending_page_final_attack.due) {
    return;
  }
  pending_page_final_attack.active = false;

  auto& sf = CharacterGameInstance::self_;
  if (sf.action == u"dead") {
    return;
  }
  if (pending_page_final_attack.sword ? !IsSword(sf) : !IsBlunt(sf)) {
    return;
  }
  auto rect = AfterimageGameInstance::LoadRect(sf);
  if (!rect.has_value()) {
    return;
  }
  auto cm = CharacterLogicSystem::RunAttackCheck(sf, *rect, 1);
  if (cm.data.empty()) {
    return;
  }
  const int damage =
      pending_page_final_attack.lv >= 30
          ? 270
          : 239 + pending_page_final_attack.lv;
  for (auto& mob : cm.data) {
    mob.hits = {damage};
  }
  auto hit_type = AfterimageGameInstance::LoadHitType(sf);
  std::string effect =
      "Afterimage/hit.img/" + std::string(hit_type.begin(), hit_type.end());
  auto cat = SkillGameInstance::CreateAttackPayload(
      cm, sf.pos, Window::dt_time_, 0, effect, "");
  for (auto& attack : cat.payload) {
    attack->attack->type =
        static_cast<uint8_t>(DamageData::DamageColor::kFinalAttack);
  }
  ClientRequest::SendToHost(cat);
}

[[maybe_unused]] static const bool r = [] {
  Mastery(1200000, true);
  Mastery(1200001, false);
  FinalAttackPassive(1201000);
  FinalAttackPassive(1201001);
  Booster(1201002, true);
  Booster(1201003, false);
  Threaten();
  Rush();
  return true;
}();

}  // namespace sdlms

