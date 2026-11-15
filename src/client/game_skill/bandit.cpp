#include "src/client/game/game_ball.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_mob.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_main.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <cstdint>
#include <flat_set>
#include <memory>
#include <random>
#include <string>

namespace sdlms {

namespace {

int SkillInt(const std::u16string& id, int lv, const char16_t* key,
             int32_t def) {
  return SkillGameInstance::LoadSkiLevelInt(
      id, static_cast<uint8_t>(std::max(1, lv)), key, def);
}

bool IsDagger(const GameCharacter& c) {
  if (!c.weapon.has_value()) {
    return false;
  }
  return EquipGameInstance::LoadWeaponEnum(c.weapon->id) ==
         EquipGameInstance::WeaponEnum::kDagger;
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
    ckt.payload.clear();
  }
  ServerCharacterInstance::HandleSki(ckt.ski_id, lv, ckt.payload, 0);
  ClientRequest::SendToHost(ckt);
}

void StunMob(uint32_t mob_index, uint64_t delay, uint64_t duration) {
  fbs::ClientMobDebuffT cmb;
  cmb.map_id = SceneSystemInstance::map_id_;
  auto md = std::make_unique<fbs::MobDebuffT>();
  md->mob_index = mob_index;
  md->payload = std::make_unique<fbs::StateT>();
  md->payload->val = static_cast<int64_t>(GameDebuff::DebuffType::kDizz);
  md->payload->delay = delay;
  md->payload->time = delay + duration;
  cmb.payload.push_back(std::move(md));
  ServerMobInstance::HandleDebuff(0, cmb);
}

// 095 Steal：偷取成功时从怪物掉落表中掉出一件物品。
void DropStolenItem(const GameMob& mob, uint64_t delay) {
  auto dts = ServerMobInstance::CreateMobDts(mob.id, mob.pos.x, mob.pos.y,
                                             mob.page);
  if (dts.empty()) {
    return;
  }
  std::uniform_int_distribution<size_t> pick(0, dts.size() - 1);
  const size_t index = pick(RandomGameInstance::gen_);
  const uint32_t map_id = SceneSystemInstance::map_id_;
  auto drops = ServerDropInstance::CreateDts({dts[index]}, map_id);
  if (drops.empty()) {
    return;
  }
  // 偷取物品在攻击命中 delay 后才掉落。
  for (const auto& dt : drops) {
    ServerDropInstance::SaveDelayDrop(map_id, delay, *dt);
  }
}

// 4200000 Dagger Mastery
void DaggerMastery() {
  GameSkill g_skill;
  g_skill.id = u"4200000";
  g_skill.passive = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    if (!IsDagger(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    CharacterStatGameInstance::SetSkiPro(
        SkillInt(u"4200000", lv, u"mastery", 0));
    CharacterStatGameInstance::ApplyPassiveCritRate(static_cast<uint32_t>(
        std::max(0, SkillInt(u"4200000", lv, u"x", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4200001 Nimble Recovery（被动）：095 在闪避成功时触发。
void NimbleRecovery() {
  GameSkill g_skill;
  g_skill.id = u"4200001";
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4201000 Dagger Booster
int booster_base_speed = 0;
bool booster_active = false;

void EndBooster() {
  if (!booster_active) {
    return;
  }
  booster_active = false;
  CharacterGameInstance::self_.attack_speed = booster_base_speed;
}

void DaggerBooster() {
  GameSkill g_skill;
  g_skill.id = u"4201000";
  g_skill.can_use = [](int ski_lv) {
    const auto& sf = CharacterGameInstance::self_;
    return IsDagger(sf) &&
           JobSkillGameInstance::LoadSkiLevel(u"4200000") >= 5 &&
           CharacterStatGameInstance::GetHp() >
               SkillInt(u"4201000", ski_lv, u"hpCon", 0);
  };
  g_skill.use = [g_skill](uint64_t, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (!IsDagger(sf)) {
      return;
    }
    const int lv = std::clamp(ski_lv, 1, 20);
    const int hp_con = SkillInt(u"4201000", lv, u"hpCon", 0);
    if (CharacterStatGameInstance::GetHp() <= hp_con) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"alert2");
    EndBooster();
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"4201000"; });
    CharacterStatGameInstance::SetHp(
        CharacterStatGameInstance::GetHp() - hp_con);
    booster_base_speed = sf.attack_speed;
    booster_active = true;
    sf.attack_speed += SkillInt(u"4201000", lv, u"x", -2);

    const int time_sec =
        std::max(1, SkillInt(u"4201000", lv, u"time", 1));
    g_skill.lv = static_cast<uint8_t>(lv);
    g_skill.duration = static_cast<uint64_t>(time_sec) * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    g_skill.end = []() {
      std::erase_if(SkillGameInstance::ski_,
                    [](const GameSkill& s) { return s.id == u"4201000"; });
      EndBooster();
      SendBuffState(4201000, 0);
      CharacterStatGameInstance::Update();
    };
    SkillGameInstance::ski_.push_back(g_skill);
    SendBuffState(4201000, 1);
    SendSkill(4201000, lv, fbs::ClientCharacterAttackT{});
    SkillGameInstance::Skis()[u"4201000"].cd = Window::dt_now_ + 1000;
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4201001 Haste
void Haste() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"4201001";
  g_skill.can_use = [](int) {
    return IsDagger(CharacterGameInstance::self_);
  };
  g_skill.buff_stat = [](int lv, SkillBuffStat& stat) {
    stat.speed = std::max(0, SkillInt(u"4201001", lv, u"indieSpeed", 0));
    stat.jump = std::max(0, SkillInt(u"4201001", lv, u"indieJump", 0));
  };
  g_skill.end = []() {
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"4201001"; });
    CharacterStatGameInstance::Update();
    SendBuffState(4201001, 0);
  };
  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    if (!IsDagger(sf)) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"alert2");
    g_skill.end();
    g_skill.lv = static_cast<uint8_t>(std::clamp(ski_lv, 1, 20));
    g_skill.duration =
        static_cast<uint64_t>(std::max(
            1, SkillInt(u"4201001", g_skill.lv, u"time", 100))) *
        1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    SkillGameInstance::ski_.push_back(g_skill);
    CharacterStatGameInstance::Update();
    SendBuffState(4201001, 1);
    if (client_id != 0) {
      return;
    }
    auto r = SkillGameInstance::LoadSkiR(u"4201001");
    auto players = CharacterLogicSystem::RunBuffCheck(sf, r);
    auto ckt =
        SkillGameInstance::CreateSkillPayload(players, 4201001, g_skill.lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, g_skill.lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
    SkillGameInstance::Skis()[u"4201001"].cd = Window::dt_now_ + 1000;
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4201002 Steal
std::flat_set<uint32_t> stolen_mobs;

void Steal() {
  GameSkill g_skill;
  g_skill.id = u"4201002";
  g_skill.can_use = [](int) {
    return IsDagger(CharacterGameInstance::self_) &&
           JobSkillGameInstance::LoadSkiLevel(u"4201001") >= 5;
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    static bool in_use = false;
    if (in_use) {
      return;
    }
    in_use = true;
    struct Guard {
      bool& flag;
      ~Guard() { flag = false; }
    } guard{in_use};

    auto& sf = CharacterGameInstance::self_;
    if (!IsDagger(sf)) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"swingO2");

    // 095：Steal 出手时无论是否偷到，都会在人物处释放影子特效 effect0。
    const int damage =
        std::max(0, SkillInt(u"4201002", ski_lv, u"damage", 120));
    const int mob_count =
        std::max(1, SkillInt(u"4201002", ski_lv, u"mobCount", 2));
    const int prop =
        std::clamp(SkillInt(u"4201002", ski_lv, u"prop", 30), 0, 100);
    const int stun_ms =
        std::max(0, SkillInt(u"4201002", ski_lv, u"subTime", 1500));
    const int time_sec =
        std::max(1, SkillInt(u"4201002", ski_lv, u"time", 30));
    const int pad =
        std::max(0, SkillInt(u"4201002", ski_lv, u"indiePad", 5));

    auto rect = SkillGameInstance::LoadSkiR(u"4201002");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, rect, mob_count);
    for (auto& mob : cm.data) {
      mob.hits = {damage};
    }

    const auto delay = SkillGameInstance::LoadSkiTime(sf);
    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      bool stole = false;
      for (auto& mob : cm.data) {
        if (stolen_mobs.contains(mob.mob.index)) {
          continue;
        }
        std::uniform_int_distribution<int> roll(0, 99);
        if (roll(RandomGameInstance::gen_) < prop) {
          stolen_mobs.insert(mob.mob.index);
          stole = true;
          if (stun_ms > 0) {
            StunMob(mob.mob.index, delay,
                    static_cast<uint64_t>(stun_ms));
          }
          DropStolenItem(mob.mob, delay);
        }
      }

      // 095：攻击命中默认使用 Afterimage 的 hit 特效。
      auto hit_type = AfterimageGameInstance::LoadHitType(sf);
      const std::string hit_effect =
          "Afterimage/hit.img/" +
          std::string(hit_type.begin(), hit_type.end());
      cat = SkillGameInstance::CreateAttackPayload(
          cm, sf.pos, delay, 0, hit_effect, "");
      ClientRequest::SendToHost(cat);

      if (stole) {
        // 095：affected 通过技能包同步给地图内所有玩家。
        std::vector<uint64_t> affected{ServerMain::local_addr_};
        auto ckt = SkillGameInstance::CreateSkillPayload(
            affected, 4201002, static_cast<uint8_t>(ski_lv));
        ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
        ClientRequest::SendToHost(ckt);
        if (pad > 0) {
          GameSkill buff;
          buff.id = u"4201002";
          buff.lv = static_cast<uint8_t>(std::clamp(ski_lv, 1, 30));
          buff.duration = static_cast<uint64_t>(time_sec) * 1000;
          buff.destroy = Window::dt_now_ + buff.duration;
          buff.buff_stat = [pad](int, SkillBuffStat& stat) {
            stat.pad += pad;
          };
          buff.end = []() {
            std::erase_if(SkillGameInstance::ski_,
                          [](const GameSkill& s) {
                            return s.id == u"4201002";
                          });
            CharacterStatGameInstance::Update();
            SendBuffState(4201002, 0);
          };
          std::erase_if(SkillGameInstance::ski_,
                        [](const GameSkill& s) { return s.id == u"4201002"; });
          SkillGameInstance::ski_.push_back(buff);
          CharacterStatGameInstance::Update();
          SendBuffState(4201002, 1);
        }
      }
    }
    SendSkill(4201002, ski_lv, cat, false);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 4201003 Savage Blow
void SavageBlow() {
  GameSkill g_skill;
  g_skill.id = u"4201003";
  g_skill.can_use = [](int) { return IsDagger(CharacterGameInstance::self_); };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsDagger(sf)) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"savage");

    const int attack_count =
        std::max(1, SkillGameInstance::LoadSkiAtkCount(
                        u"4201003", static_cast<uint8_t>(ski_lv)));
    const int damage =
        std::max(1, SkillInt(u"4201003", ski_lv, u"damage", 65));
    const int mob_count =
        std::max(1, SkillInt(u"4201003", ski_lv, u"mobCount", 1));

    auto rect = SkillGameInstance::LoadSkiR(u"4201003");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, rect, mob_count);
    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      const auto delay = SkillGameInstance::LoadSkiTime(sf);
      for (auto& mob : cm.data) {
        mob.hits.assign(static_cast<size_t>(attack_count), damage);
      }
      cat = SkillGameInstance::CreateAttackPayload(
          cm, sf.pos, delay, 120, "", "Skill.img/4201003/Hit");
      // 095 TryDoingMeleeAttack：所有连击放在同一个攻击包内，
      size_t index = 0;
      for (const auto& mob : cm.data) {
        for (int i = 0; i < attack_count && index < cat.payload.size();
             ++i, ++index) {
          cat.payload[index]->attack->delay =
              delay + 120 * i;
          cat.payload[index]->effect = "420.img/skill/4201003/hit/" +
                                       std::to_string(i % 2);
        }
      }
      ClientRequest::SendToHost(cat);
    }
    SendSkill(4201003, ski_lv, cat, false);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

}  // namespace

[[maybe_unused]] static const bool r = [] {
  DaggerMastery();
  NimbleRecovery();
  DaggerBooster();
  Haste();
  Steal();
  SavageBlow();
  return true;
}();

}  // namespace sdlms
