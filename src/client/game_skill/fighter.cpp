#include "src/client/game/game_effect.h"
#include "src/client/game_skill/fighter_final_attack.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game/game_foothold.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"

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

int SkillInt(const char16_t* id, int lv, const char16_t* key, int fallback) {
  return SkillGameInstance::LoadSkiLevelInt(
      std::u16string(id), static_cast<uint8_t>(std::max(1, lv)), key,
      fallback);
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

bool IsAxe(const GameCharacter& c) {
  if (!c.weapon.has_value()) {
    return false;
  }
  auto w = EquipGameInstance::LoadWeaponEnum(c.weapon->id);
  return w == EquipGameInstance::WeaponEnum::kAxe1H ||
         w == EquipGameInstance::WeaponEnum::kAxe2H;
}

void SendBuffState(int ski_id, int sub_val) {
  fbs::StateT st;
  st.state = fbs::StateEnum_BUFF_SKILL;
  st.val = ski_id;
  st.sub_val = sub_val;
  CharacterLogicSystem::ccs_.payload.push_back(
      std::make_unique<fbs::StateT>(st));
}

void EraseSkill(const std::u16string& id) {
  auto& ski = SkillGameInstance::ski_;
  auto it = std::ranges::find_if(
      ski, [&id](const GameSkill& s) { return s.id == id; });
  if (it != ski.end()) {
    ski.erase(it);
  }
}

void StopSkill(const std::u16string& id) {
  EraseSkill(id);
  CharacterStatGameInstance::Update();
  SendBuffState(U16ToInt(id), 0);
}

void SendAttack(uint32_t id, int lv, fbs::ClientCharacterAttackT& cat,
                bool send_hit_effects = true) {
  if (!cat.payload.empty()) {
    ClientRequest::SendToHost(cat);
  }
  auto ckt = SkillGameInstance::CreateSkillPayload(cat, id, lv);
  if (!send_hit_effects) {
    ckt.payload.clear();
  }
  ServerCharacterInstance::HandleSki(ckt.ski_id, lv, ckt.payload, 0);
  ClientRequest::SendToHost(ckt);
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

void Booster(uint32_t id, bool sword) {
  GameSkill g_skill;
  g_skill.id = U16(id);
  g_skill.fall = true;
  g_skill.can_use = [sword](int) {
    const auto& sf = CharacterGameInstance::self_;
    return sword ? IsSword(sf) : IsAxe(sf);
  };
  g_skill.use = [g_skill, sword, id](uint64_t, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (sword ? !IsSword(sf) : !IsAxe(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    const int hp_con = SkillInt(
        sword ? u"1101002" : u"1101003", lv, u"hpCon", 0);
    if (CharacterStatGameInstance::GetHp() <= hp_con) {
      return;
    }
    // 关闭旧 Booster，避免叠加。
    EndBooster();
    StopSkill(U16(id));

    CharacterStatGameInstance::SetHp(
        CharacterStatGameInstance::GetHp() - hp_con);
    booster_base_speed = sf.attack_speed;
    booster_active = true;
    sf.attack_speed += SkillInt(
        sword ? u"1101002" : u"1101003", lv, u"x", -2);

    const int time_sec = std::max(
        1, SkillInt(sword ? u"1101002" : u"1101003", lv, u"time", 1));
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
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void FinalAttackPassive(uint32_t id) {
  GameSkill g_skill;
  g_skill.id = U16(id);
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void Rage() {
  GameSkill g_skill;
  g_skill.id = u"1101004";
  g_skill.buff_stat = [](int lv, SkillBuffStat& stat) {
    stat.pad += SkillInt(u"1101004", lv, u"indiePad", 0);
    stat.pdd += SkillInt(u"1101004", lv, u"indiePdd", 0);
  };
  g_skill.use = [g_skill](uint64_t, int ski_lv) mutable {
    const int lv = std::clamp(ski_lv, 1, 20);
    StopSkill(g_skill.id);
    g_skill.lv = static_cast<uint8_t>(lv);
    const int time_sec =
        std::max(1, SkillInt(u"1101004", lv, u"time", 1));
    g_skill.duration = static_cast<uint64_t>(time_sec) * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    g_skill.end = []() {
      EraseSkill(u"1101004");
      SendBuffState(1101004, 0);
      CharacterStatGameInstance::Update();
    };
    SkillGameInstance::ski_.push_back(g_skill);
    SendBuffState(1101004, 1);
    CharacterStatGameInstance::Update();
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void Rush() {
  GameSkill g_skill;
  g_skill.id = u"1101005";
  g_skill.fall = true;
  g_skill.can_use = [](int) {
    const auto& sf = CharacterGameInstance::self_;
    return IsSword(sf) || IsAxe(sf);
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsSword(sf) && !IsAxe(sf)) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"rushO");
    const int mob_count =
        std::max(1, SkillInt(u"1101005", ski_lv, u"mobCount", 4));
    auto g_r = SkillGameInstance::LoadSkiR(u"1101005");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r, mob_count);
    const int damage =
        std::max(0, SkillInt(u"1101005", ski_lv, u"damage", 60));
    for (auto& mob : cm.data) {
      mob.hits = {damage};
    }
    fbs::ClientCharacterAttackT cat;
    const int range =
        std::max(0, SkillInt(u"1101005", ski_lv, u"range", 130));
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      auto hit_type = AfterimageGameInstance::LoadHitType(sf);
      std::string effect =
          "Afterimage/hit.img/" +
          std::string(hit_type.begin(), hit_type.end());
      cat = SkillGameInstance::CreateAttackPayload(
          cm, sf.pos, delay, 0, effect, "", static_cast<float>(range));
      for (auto& attack : cat.payload) {
        attack->attack->knockback_speed = 1.6f;
      }
      RegisterFinalAttack(sf, delay);
    }
    // 095：Rush 沿脚下 foothold 快速推进，而不是瞬移；
    // 到达当前 foothold 末端就停止，避免冲进空中。
    struct RushState {
      float remaining = 0.0f;
      float dir = 0.0f;
    };
    auto rush = std::make_shared<RushState>();
    rush->remaining = static_cast<float>(range);
    rush->dir = sf.flip ? 1.0f : -1.0f;
    // 095：Rush 突进期间人物不会被怪物攻击打断。
    CharacterLogicSystem::self_invincible_cooldown_ = std::max(
        CharacterLogicSystem::self_invincible_cooldown_,
        Window::dt_now_ + 100);

    GameSkill move;
    move.id = u"1101005";
    move.destroy = Window::dt_now_ + 1000;
    move.frame = [rush]() {
      if (rush->remaining > 0.0f) {
        CharacterLogicSystem::self_invincible_cooldown_ = std::max(
            CharacterLogicSystem::self_invincible_cooldown_,
            Window::dt_now_ + 100);
      }
      auto& sf = CharacterGameInstance::self_;
      // 单帧最多跨越 8 个相连 foothold，避免异常连接导致死循环。
      for (int guard = 0; rush->remaining > 0.0f && guard < 8; ++guard) {
        const int32_t fh_id = CharacterLogicSystem::self_fh_;
        auto fh_it = FootholdGameInstance::data_.find(fh_id);
        if (fh_id == 0 || fh_it == FootholdGameInstance::data_.end()) {
          rush->remaining = 0.0f;
          return;
        }
        const auto& fh = fh_it->second;
        const float min_x =
            static_cast<float>(std::min(fh.x1, fh.x2));
        const float max_x =
            static_cast<float>(std::max(fh.x1, fh.x2));
        const float endpoint = rush->dir > 0.0f ? max_x : min_x;

        const float step = std::min(
            rush->remaining, 700.0f * Window::delta_time_ / 1000.0f);
        const float old_x = sf.pos.x;
        float target_x = std::clamp(
            old_x + rush->dir * step, min_x, max_x);
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
        // 还没走到当前 foothold 末端，下一帧继续。
        if (std::fabs(sf.pos.x - endpoint) > 0.01f) {
          return;
        }

        const int32_t next_id =
            rush->dir > 0.0f ? fh.next : fh.prev;
        if (next_id == 0 || next_id == fh_id) {
          rush->remaining = 0.0f;
          return;
        }
        // 同时检查 prev/next，选一个在前进方向延伸且距离最近的连接点。
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
            const float dx = sf.pos.x - p.x;
            const float dy = sf.pos.y - p.y;
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
    move.end = []() { EraseSkill(u"1101005"); };
    SkillGameInstance::ski_.push_back(move);

    SendAttack(1101005, ski_lv, cat, false);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void SwordMastery() {
  GameSkill g_skill;
  g_skill.id = u"1100000";
  g_skill.passive = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    if (!IsSword(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    CharacterStatGameInstance::SetSkiPro(
        SkillInt(u"1100000", lv, u"mastery", 0));
    CharacterStatGameInstance::SetSkiDef(
        SkillInt(u"1100000", lv, u"x", 0));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void AxeMastery() {
  GameSkill g_skill;
  g_skill.id = u"1100001";
  g_skill.passive = [](int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    const bool axe = IsAxe(sf);
    const int lv = std::clamp(ski_lv, 1, 20);
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"1100001"; });
    if (!axe) {
      if (it != ski.end()) {
        ski.erase(it);
      }
      return;
    }
    CharacterStatGameInstance::SetSkiPro(
        SkillInt(u"1100001", lv, u"mastery", 0));
    if (it != ski.end()) {
      return;
    }
    GameSkill bleed;
    bleed.id = u"1100001";
    bleed.action = [](GameSkill*, std::u16string skill_id) {
      if (!skill_id.empty()) {
        return;
      }
      const int lv = JobSkillGameInstance::LoadSkiLevel(u"1100001");
      if (lv <= 0) {
        return;
      }
      const int prop =
          SkillInt(u"1100001", lv, u"prop", 0);
      std::uniform_int_distribution<int> roll(0, 99);
      if (roll(RandomGameInstance::gen_) >= prop) {
        return;
      }
      ExtraHit(CharacterGameInstance::self_,
               SkillInt(u"1100001", lv, u"dot", 0), 1);
    };
    ski.push_back(bleed);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  SwordMastery();
  AxeMastery();
  FinalAttackPassive(1101000);
  FinalAttackPassive(1101001);
  Booster(1101002, true);
  Booster(1101003, false);
  Rage();
  Rush();
  return true;
}();

}  // namespace

namespace {

struct PendingFinalAttack {
  bool active = false;
  uint64_t due = 0;
  uint8_t lv = 0;
  bool sword = false;
};

PendingFinalAttack pending_final_attack;

}  // namespace

void RegisterFinalAttack(GameCharacter& g_character, uint64_t attack_delay) {
  const bool sword = IsSword(g_character);
  const bool axe = IsAxe(g_character);
  if (!sword && !axe) {
    return;
  }
  const uint32_t id = sword ? 1101000 : 1101001;
  const int lv = JobSkillGameInstance::LoadSkiLevel(U16(id));
  if (lv <= 0) {
    return;
  }
  const int prop = SkillInt(sword ? u"1101000" : u"1101001", lv, u"prop", 0);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) >= prop) {
    return;
  }
  pending_final_attack.active = true;
  pending_final_attack.due = attack_delay + 150;
  pending_final_attack.lv = static_cast<uint8_t>(lv);
  pending_final_attack.sword = sword;
}

void UpdateFinalAttack() {
  if (!pending_final_attack.active ||
      Window::dt_time_ < pending_final_attack.due) {
    return;
  }
  pending_final_attack.active = false;

  auto& sf = CharacterGameInstance::self_;
  if (sf.action == u"dead") {
    return;
  }
  if (pending_final_attack.sword ? !IsSword(sf) : !IsAxe(sf)) {
    return;
  }
  auto rect = AfterimageGameInstance::LoadRect(sf);
  if (!rect.has_value()) {
    return;
  }
  auto cm = CharacterLogicSystem::RunAttackCheck(sf, *rect, 3);
  if (cm.data.empty()) {
    return;
  }
  const int damage = 109 + pending_final_attack.lv;
  for (auto& mob : cm.data) {
    mob.hits = {damage};
  }
  auto hit_type = AfterimageGameInstance::LoadHitType(sf);
  std::string effect =
      "Afterimage/hit.img/" +
      std::string(hit_type.begin(), hit_type.end());
  auto cat = SkillGameInstance::CreateAttackPayload(
      cm, sf.pos, Window::dt_time_, 0, effect, "");
  for (auto& attack : cat.payload) {
    attack->attack->type =
        static_cast<uint8_t>(DamageData::DamageColor::kFinalAttack);
  }
  ClientRequest::SendToHost(cat);
}

}  // namespace sdlms
