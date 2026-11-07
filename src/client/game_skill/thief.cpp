
#include "src/client/game/game_character.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_mob.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_mob_instance.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <cstdint>
#include <memory>
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

bool IsDagger(const GameCharacter& c) {
  if (!c.weapon.has_value()) {
    return false;
  }
  return EquipGameInstance::LoadWeaponEnum(c.weapon->id) ==
         EquipGameInstance::WeaponEnum::kDagger;
}

bool IsBuffActive(const char16_t* id) {
  const std::u16string skill_id(id);
  return std::ranges::any_of(SkillGameInstance::ski_, [&](const GameSkill& s) {
    return s.id == skill_id;
  });
}

void SendBuffState(int ski_id, int sub_val) {
  fbs::StateT st;
  st.state = fbs::StateEnum_BUFF_SKILL;
  st.val = ski_id;
  st.sub_val = sub_val;
  CharacterLogicSystem::ccs_.payload.push_back(
      std::make_unique<fbs::StateT>(st));
}

void SendSkill(uint32_t id, int lv, const fbs::ClientCharacterAttackT& cat,
               bool send_hit_effects = true) {
  auto ckt = SkillGameInstance::CreateSkillPayload(cat, id, lv);
  if (!send_hit_effects) {
    // 由攻击 payload 自己指定 hit/0、hit/1，避免再生成通用 hit/0。
    ckt.payload.clear();
  }
  ServerCharacterInstance::HandleSki(ckt.ski_id, lv, ckt.payload, 0);
  ClientRequest::SendToHost(ckt);
}

}  // namespace

// 4000000 Nimble Body：WZ accX / evaX。
static void NimbleBody() {
  GameSkill g_skill;
  g_skill.id = u"4000000";
  g_skill.passive = [](int ski_lv) {
    const int lv = std::clamp(ski_lv, 1, 15);
    CharacterStatGameInstance::ApplyPassiveAcc(static_cast<uint32_t>(
        std::max(0, SkillInt(u"4000000", lv, u"accX", 0))));
    CharacterStatGameInstance::ApplyPassiveAvoid(static_cast<uint32_t>(
        std::max(0, SkillInt(u"4000000", lv, u"evaX", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4000001 Keen Eyes：WZ range，只增加飞镖类武器射程。
static void KeenEyes() {
  GameSkill g_skill;
  g_skill.id = u"4000001";
  g_skill.passive = [](int ski_lv) {
    const int lv = std::clamp(ski_lv, 1, 15);
    CharacterStatGameInstance::ApplyPassiveThrowRange(static_cast<uint32_t>(
        std::max(0, SkillInt(u"4000001", lv, u"range", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4001001 Dark Sight：WZ speed 为移动速度惩罚；开启/关闭切换。
static void DarkSight() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"4001001";
  g_skill.skip_mp = []() { return IsBuffActive(u"4001001"); };
  // WZ 的 speed 是减速量（正值），Buff 表里移速是加成，需要取负。
  g_skill.buff_stat = [](int lv, SkillBuffStat& stat) {
    stat.speed = -std::max(0, SkillInt(u"4001001", lv, u"speed", 0));
  };

  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"4001001"; });
    if (it != ski.end()) {
      ski.erase(it);
      CharacterStatGameInstance::Update();
      CharacterGameInstance::self_.color = {255, 255, 255, 255};
      SendBuffState(4001001, 0);
    }
  };

  g_skill.state = [](GameCharacter* c, int index) {
    if (index == 1) {
      c->color = {255, 255, 255, 128};
    } else {
      c->color = {255, 255, 255, 255};
    }
  };
  g_skill.frame = [g_skill]() {
    auto& self = CharacterGameInstance::self_;
    self.color = {255, 255, 255, 128};
    auto type = CharacterLogicSystem::LoadActionType(self);
    switch (type) {
      case CharacterLogicSystem::ActionEnum::kAttack:
      case CharacterLogicSystem::ActionEnum::kSkill: {
        g_skill.end();
        break;
      }
      default: {
        CharacterLogicSystem::self_invincible_cooldown_ = Window::dt_now_ + 100;
        break;
      }
    }
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    SkillGameInstance::Skis()[u"2001000"].cd = Window::dt_now_ + 1000;
    if (IsBuffActive(u"4001001")) {
      g_skill.end();
      g_skill.state(&sf, 0);
      return;
    }

    g_skill.lv = static_cast<uint8_t>(ski_lv);
    SkillGameInstance::ski_.push_back(g_skill);
    CharacterStatGameInstance::Update();

    SendSkill(4001001, ski_lv, fbs::ClientCharacterAttackT{});
    SendBuffState(4001001, 1);
    g_skill.state(&sf, 0);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4001000 Disorder：伤害 100%，并降低怪物攻击力/物防。
static void Disorder() {
  GameSkill g_skill;
  g_skill.id = u"4001000";
  g_skill.fall = true;
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAttackAction(sf, false);

    const int damage =
        std::max(0, SkillInt(u"4001000", ski_lv, u"damage", 100));
    const int time_sec = std::max(1, SkillInt(u"4001000", ski_lv, u"time", 10));
    const int atk_down = std::max(0, SkillInt(u"4001000", ski_lv, u"x", 0));
    const int def_down = std::max(0, SkillInt(u"4001000", ski_lv, u"y", 0));

    auto g_r = SkillGameInstance::LoadSkiR(u"4001000");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r, 1);
    if (!cm.data.empty()) {
      cm.data.resize(1);
      cm.data[0].hits = {damage};
    }

    fbs::ClientCharacterAttackT cat;
    const auto attack_delay = SkillGameInstance::LoadSkiTime(sf);
    if (!cm.data.empty()) {
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, attack_delay);
      ClientRequest::SendToHost(cat);
    }

    if (!cm.data.empty()) {
      const auto mob_index = cm.data[0].mob.index;
      auto it = MobGameInstance::data_.find(mob_index);
      const bool already =
          it != MobGameInstance::data_.end() &&
          it->second.mob.debuffs.contains(GameDebuff::DebuffType::kDisorder);
      if (!already) {
        fbs::ClientMobDebuffT cmb;
        cmb.map_id = SceneSystemInstance::map_id_;
        auto md = std::make_unique<fbs::MobDebuffT>();
        md->mob_index = mob_index;
        md->payload = std::make_unique<fbs::StateT>();
        md->payload->val =
            static_cast<int64_t>(GameDebuff::DebuffType::kDisorder);
        // 低 16 位攻击力降低，高 16 位物防降低。
        md->payload->sub_val = static_cast<int64_t>(atk_down & 0xFFFF) |
                               (static_cast<int64_t>(def_down & 0xFFFF) << 16);
        md->payload->delay = attack_delay;
        md->payload->time =
            attack_delay + static_cast<uint64_t>(time_sec) * 1000;
        cmb.payload.push_back(std::move(md));
        ServerMobInstance::HandleDebuff(0, cmb);
      }
    }

    SendSkill(4001000, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4001002 Double Stab：WZ attackCount 次，damage 为每次伤害。
static void DoubleStab() {
  GameSkill g_skill;
  g_skill.id = u"4001002";
  g_skill.fall = true;
  g_skill.can_use = [](int) { return IsDagger(CharacterGameInstance::self_); };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsDagger(sf)) {
      return;
    }
    CharacterLogicSystem::RunAttackAction(sf, false);

    const int attack_count =
        std::max(1, SkillInt(u"4001002", ski_lv, u"attackCount", 2));
    const int mob_count =
        std::max(1, SkillInt(u"4001002", ski_lv, u"mobCount", 1));
    const int damage = std::max(0, SkillInt(u"4001002", ski_lv, u"damage", 80));

    auto g_r = SkillGameInstance::LoadSkiR(u"4001002");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r, mob_count);
    for (auto& mob : cm.data) {
      mob.hits.assign(static_cast<size_t>(attack_count), damage);
    }

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay, 120, "",
                                                   "");
      // 095：DoubleStab 每一段命中分别播放 hit/0、hit/1。
      for (size_t i = 0; i < cat.payload.size(); ++i) {
        cat.payload[i]->effect = "400.img/skill/4001002/hit/" +
                                 std::to_string(i % 2);
      }
      ClientRequest::SendToHost(cat);
    }
    SendSkill(4001002, ski_lv, cat, false);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4001003 Lucky Seven：WZ bulletCount 个飞镖，每个 damage%。
static void LuckySeven() {
  GameSkill g_skill;
  g_skill.id = u"4001003";
  g_skill.fall = true;
  g_skill.can_use = [](int) {
    return IsClaw(CharacterGameInstance::self_) &&
           PackageGameInstance::LoadActiveBall() != nullptr;
  };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsClaw(sf)) {
      return;
    }
    auto ball = PackageGameInstance::LoadActiveBall();
    if (ball == nullptr) {
      return;
    }

    const int range =
        350 + static_cast<int>(CharacterStatGameInstance::GetThrowRangeBonus());
    const int mob_count =
        std::max(1, SkillInt(u"4001003", ski_lv, u"mobCount", 1));
    const int bullet_count =
        std::max(1, SkillInt(u"4001003", ski_lv, u"bulletCount", 2));
    const int attack_count =
        std::max(1, SkillInt(u"4001003", ski_lv, u"attackCount", 1));
    const int hit_count = bullet_count * attack_count;
    const int damage = std::max(0, SkillInt(u"4001003", ski_lv, u"damage", 0));

    CharacterLogicSystem::RunAttackAction(sf, true);
    GameTriangle tri = {{
        SDL_FPoint{static_cast<float>(-range), -100},
        SDL_FPoint{static_cast<float>(-range), 100},
        SDL_FPoint{0, -28},
    }};
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri, mob_count);
    for (auto& mob : cm.data) {
      mob.hits.assign(static_cast<size_t>(hit_count), damage);
    }

    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto pos = sf.pos;
    pos.y -= 28;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? range : -range;
    std::u16string path = u"Consume/0207.img/" + (*ball)->id + u"/bullet";
    auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay,
                                                   sf.page, 700, path);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d, 120, "", "");
      ClientRequest::SendToHost(cat);
    }

    // 095：每颗飞镖从同一个出手点发射，第二颗延后 BulletDelay(120ms)。
    for (int i = 0; i < bullet_count; ++i) {
      cct.payload->ball->y1 = sf.pos.y - 28;
      cct.payload->ball->delay = delay + i * 120;
      ClientRequest::SendToHost(cct);
      ServerBallInstance::HandleServerB(cct.payload);
    }
    ItemGameInstance::DecItemNum(*ball, bullet_count);
    SendSkill(4001003, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  NimbleBody();
  KeenEyes();
  Disorder();
  DarkSight();
  DoubleStab();
  LuckySeven();
  return true;
}();

}  // namespace sdlms
