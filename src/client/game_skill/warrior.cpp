#include "src/client/game_skill/fighter_final_attack.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <any>
#include <cstdint>
#include <memory>

namespace sdlms {

namespace {

int32_t LoadDamage(const char16_t* id, int lv) {
  return SkillGameInstance::LoadSkiLevelInt(
      std::u16string(id), static_cast<uint8_t>(std::max(1, lv)), u"damage", 0);
}

int32_t LoadHpCon(int lv) {
  return SkillGameInstance::LoadSkiLevelInt(
      u"1001002", static_cast<uint8_t>(std::clamp(lv, 1, 20)), u"hpCon", 0);
}

void SendBuffState(int ski_id, int sub_val) {
  fbs::StateT st;
  st.state = fbs::StateEnum_BUFF_SKILL;
  st.val = ski_id;
  st.sub_val = sub_val;
  CharacterLogicSystem::ccs_.payload.push_back(
      std::make_unique<fbs::StateT>(st));
}

}  // namespace

// 1001001 Power Strike：160% -> 260%，单目标。
static void PowerStrike() {
  GameSkill g_skill;
  g_skill.id = u"1001001";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAttackAction(sf, false);

    auto g_r_opt = AfterimageGameInstance::LoadRect(sf);
    if (!g_r_opt.has_value()) {
      return;
    }
    const int32_t damage =
        std::max<int32_t>(0, LoadDamage(u"1001001", ski_lv));
    const int attack_count = std::max(
        1, SkillGameInstance::LoadSkiAtkCount(
               u"1001001", static_cast<uint8_t>(std::max(1, ski_lv))));
    const int mob_count = std::max(
        1, SkillGameInstance::LoadSkiMobCount(
               u"1001001", static_cast<uint8_t>(std::max(1, ski_lv))));
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r_opt.value(), mob_count);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      cm.data[0].hits.assign(static_cast<size_t>(attack_count), damage);
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay);
      ClientRequest::SendToHost(cat);
      RegisterFinalAttack(sf, delay);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 1001001, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 1001002 Slash Blast：消耗 HP+MP，最多 6 个目标，70% -> 130%。
static void SlashBlast() {
  GameSkill g_skill;
  g_skill.id = u"1001002";
  g_skill.can_use = [](int ski_lv) {
    const int32_t hp_con = LoadHpCon(ski_lv);
    return CharacterStatGameInstance::GetHp() > hp_con;
  };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    const int32_t hp_con = LoadHpCon(ski_lv);
    if (CharacterStatGameInstance::GetHp() <= hp_con) {
      return;
    }

    CharacterLogicSystem::RunAttackAction(sf, false);
    CharacterStatGameInstance::SetHp(CharacterStatGameInstance::GetHp() -
                                     hp_con);

    auto g_r = SkillGameInstance::LoadSkiR(u"1001002");
    // WZ common/mobCount = 4：Slash Blast 最多攻击 4 个目标。
    constexpr int kMobCount = 4;
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r, kMobCount);

    const int32_t damage =
        std::max<int32_t>(0, LoadDamage(u"1001002", ski_lv));
    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      for (auto& mob : cm.data) {
        mob.hits = {damage};
      }
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay);
      ClientRequest::SendToHost(cat);
      RegisterFinalAttack(sf, delay);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 1001002, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 1001000 Iron Body：WZ indiePddR 5% -> 25%，time 300 -> 600 秒。
static void IronBody() {
  GameSkill g_skill;
  g_skill.id = u"1001000";
  g_skill.climb = true;
  g_skill.buff_stat = [](int lv, SkillBuffStat& stat) {
    stat.pdd_rate = std::max(
        0, SkillGameInstance::LoadSkiLevelInt(
               u"1001000", static_cast<uint8_t>(std::max(1, lv)),
               u"indiePddR", 0));
  };

  // 与 MagicArmor 保持一致的放大透明残影效果。
  g_skill.effect = [](SDL_FPoint p, GameEffect* e, bool f) {
    const int duration = 300;
    auto start = e->delay;
    auto dt = Window::dt_now_ - start;
    if (dt <= duration) {
      float scale = 1 + 1.2f * static_cast<float>(dt) / duration;
      auto g_character = std::any_cast<GameCharacter*>(e->data);
      auto character = *g_character;
      character.scale = scale;
      character.color.a =
          static_cast<Uint8>(192 * (1 - static_cast<float>(dt) / duration));
      CharacterRenderSystem::RenderCharacter(character);
      return true;
    }
    return false;
  };

  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"1001000"; });
    if (it != ski.end()) {
      ski.erase(it);
      CharacterStatGameInstance::Update();
      SendBuffState(1001000, 0);
    }
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& ski = SkillGameInstance::ski_;
    g_skill.end();

    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAction(sf, u"alert2");

    const int lv = std::clamp(ski_lv, 1, 20);
    g_skill.lv = static_cast<uint8_t>(lv);
    int duration_sec = SkillGameInstance::LoadSkiLevelInt(
        u"1001000", static_cast<uint8_t>(lv), u"time", 0);
    if (duration_sec <= 0) {
      duration_sec = 300;
    }
    g_skill.duration = static_cast<uint64_t>(duration_sec) * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    ski.push_back(g_skill);

    auto ckt = SkillGameInstance::CreateSkillPayload(1001000, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
    CharacterStatGameInstance::Update();
    SendBuffState(1001000, 1);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 1000000 Improved HP Recovery：每 10 秒恢复；道具 HP 恢复率读取 WZ y。
static void ImprovedHpRecovery() {
  GameSkill g_skill;
  g_skill.id = u"1000000";
  g_skill.passive = [](int ski_lv) {
    const int lv = std::clamp(ski_lv, 1, 15);
    CharacterStatGameInstance::ApplyPassiveHpRecovery(
        static_cast<uint32_t>(std::max(
            0, SkillGameInstance::LoadSkiLevelInt(
                   u"1000000", static_cast<uint8_t>(lv), u"y", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 1000001 Max HP Increase：最大 HP 百分比读取 WZ mhpR。
static void MaxHpIncrease() {
  GameSkill g_skill;
  g_skill.id = u"1000001";
  g_skill.passive = [](int ski_lv) {
    const int lv = std::clamp(ski_lv, 1, 15);
    CharacterStatGameInstance::ApplyPassiveHpRate(
        SkillGameInstance::LoadSkiLevelInt(
            u"1000001", static_cast<uint8_t>(lv), u"mhpR", 0));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 1000002 Precise Strikes：命中/暴击率读取 WZ accX / crtX。
static void PreciseStrikes() {
  GameSkill g_skill;
  g_skill.id = u"1000002";
  g_skill.passive = [](int ski_lv) {
    const int lv = std::clamp(ski_lv, 1, 15);
    CharacterStatGameInstance::ApplyPassiveAcc(
        static_cast<uint32_t>(std::max(
            0, SkillGameInstance::LoadSkiLevelInt(
                   u"1000002", static_cast<uint8_t>(lv), u"accX", 0))));
    CharacterStatGameInstance::ApplyPassiveCritRate(
        static_cast<uint32_t>(std::max(
            0, SkillGameInstance::LoadSkiLevelInt(
                   u"1000002", static_cast<uint8_t>(lv), u"crtX", 0))));
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  PowerStrike();
  SlashBlast();
  IronBody();
  ImprovedHpRecovery();
  MaxHpIncrease();
  PreciseStrikes();
  return true;
}();

}  // namespace sdlms
