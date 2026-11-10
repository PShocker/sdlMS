#include "src/client/game/game_effect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/effect_logic_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <any>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace sdlms {

namespace {

int SkillInt(const std::u16string& id, int lv, const char16_t* key,
             int32_t def) {
  return SkillGameInstance::LoadSkiLevelInt(
      id, static_cast<uint8_t>(std::max(1, lv)), key, def);
}

bool IsMagicWeapon(const GameCharacter& c) {
  auto weapon = EquipGameInstance::LoadWeaponType(c);
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

void SendSkill(uint32_t id, int lv, const fbs::ClientCharacterAttackT& cat) {
  auto ckt = SkillGameInstance::CreateSkillPayload(cat, id, lv);
  ServerCharacterInstance::HandleSki(ckt.ski_id, lv, ckt.payload, 0);
  ClientRequest::SendToHost(ckt);
}

void MpEaterPassive() {
  GameSkill g_skill;
  g_skill.id = u"2300000";
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2301000 Teleport：按方向键瞬移 WZ range。
void Teleport() {
  GameSkill g_skill;
  g_skill.id = u"2301000";
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_);
  };
  g_skill.effect = [](SDL_FPoint p, GameEffect* e, bool f) {
    e->id = u"BasicEff.img/Teleport";
    e->lv = 1;
    e->pos = SDL_FPoint{0, 0};
    if (EffectLogicSystem::RunEffect(*e)) {
      return false;
    }
    auto g_character = std::any_cast<GameCharacter*>(e->data);
    EffectRenderSystem::RenderEffect(g_character->pos, *e);
    e->id = u"2301000";
    return true;
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsMagicWeapon(sf)) {
      return;
    }
    SkillGameInstance::Skis()[u"2301000"].cd = Window::dt_now_ + 500;
    const int range = std::max(1, SkillInt(u"2301000", ski_lv, u"range", 110));
    auto up = KeyboardGameInstance::FindKeyByVal("up");
    auto down = KeyboardGameInstance::FindKeyByVal("down");
    auto left = KeyboardGameInstance::FindKeyByVal("left");
    auto right = KeyboardGameInstance::FindKeyByVal("right");

    auto x = sf.pos.x;
    auto y = sf.pos.y;
    if (Window::keyboard_state_[right]) {
      x += range;
    } else if (Window::keyboard_state_[left]) {
      x -= range;
    } else if (Window::keyboard_state_[down]) {
      y += range;
    } else if (Window::keyboard_state_[up]) {
      y -= range;
    }

    const auto& fhs = FootholdGameInstance::data_;
    auto border =
        MapInfoGameInstance::LoadMrBorder(SceneSystemInstance::map_id_);
    auto [l, t, r, b] = border;
    x = std::clamp(x, l, r);
    y = std::clamp(y, t, b);
    if (y != sf.pos.y) {
      auto ins = Physic::FallIntersectPos(sf.pos, {x, y}, fhs);
      if (!ins.empty()) {
        if (sf.pos.y < y) {
          sf.pos = ins.rbegin()->second.pos;
          CharacterLogicSystem::self_fh_ = ins.rbegin()->second.fh.id;
        } else {
          sf.pos = ins.begin()->second.pos;
          CharacterLogicSystem::self_fh_ = ins.begin()->second.fh.id;
        }
      }
    } else if (x != sf.pos.x) {
      auto ins = Physic::FallIntersectPos({x, y}, {x, y + 70}, fhs);
      if (!ins.empty()) {
        sf.pos = ins.begin()->second.pos;
        CharacterLogicSystem::self_fh_ = ins.begin()->second.fh.id;
      } else {
        auto ins2 = Physic::FallIntersectPos({x, y}, {x, y - 70}, fhs);
        if (!ins2.empty()) {
          sf.pos = ins2.rbegin()->second.pos;
          CharacterLogicSystem::self_fh_ = ins2.begin()->second.fh.id;
        }
      }
    }
    CharacterLogicSystem::self_hspeed_ = 0;
    CharacterLogicSystem::self_vspeed_ = 0;
    SendSkill(2301000, ski_lv, fbs::ClientCharacterAttackT{});
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2301001 Heal：范围恢复队伍 HP，并对范围内 undead 怪物造成治疗量伤害。
void Heal() {
  GameSkill g_skill;
  g_skill.id = u"2301001";
  g_skill.climb = true;
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsMagicWeapon(sf)) {
      return;
    }
    auto action_type = CharacterLogicSystem::LoadActionType(sf);
    if (action_type != CharacterLogicSystem::ActionEnum::kClimb) {
      CharacterLogicSystem::RunAction(sf, u"alert2");
    } else {
      CharacterLogicSystem::self_climb_animate_cooldown_ =
          Window::dt_now_ + 700;
    }
    SkillGameInstance::Skis()[u"2301001"].cd = Window::dt_now_ + 1000;

    const int heal_base = std::max(1, SkillInt(u"2301001", ski_lv, u"x", 40));
    const int heal = heal_base + CharacterStatGameInstance::GetMagic() / 2;
    CharacterStatGameInstance::SetHp(
        std::min(CharacterStatGameInstance::GetHpMax(),
                 CharacterStatGameInstance::GetHp() + heal));

    const int mob_count =
        std::max(1, SkillInt(u"2301001", ski_lv, u"mobCount", 6));
    auto rect = SkillGameInstance::LoadSkiR(u"2301001");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, rect, mob_count);
    CheckMobs damage_cm;
    for (auto& mob : cm.data) {
      auto* info = MobGameInstance::LoadMobInfo(mob.mob.id);
      auto* undead = info == nullptr ? nullptr : info->get_child(u"undead");
      if (undead == nullptr || undead->type != wz::Type::Int ||
          static_cast<wz::Property<int>*>(undead)->get() == 0) {
        continue;
      }
      mob.hits = {heal};
      mob.raw_damage = true;
      damage_cm.data.push_back(std::move(mob));
    }
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    fbs::ClientCharacterAttackT cat;
    if (!damage_cm.data.empty()) {
      cat = SkillGameInstance::CreateAttackPayload(damage_cm, sf.pos, delay);
      ClientRequest::SendToHost(cat);
    }
    SendSkill(2301001, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2301002 Invincible：减少物理伤害，并让接触伤害来源受到无属性魔法伤害。
void Invincible() {
  GameSkill g_skill;
  g_skill.id = u"2301002";
  g_skill.climb = true;
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_) &&
           JobSkillGameInstance::LoadSkiLevel(u"2301001") >= 5;
  };
  g_skill.buff_stat = [](int lv, SkillBuffStat& stat) {
    stat.pdd_rate += SkillInt(u"2301002", lv, u"x", 0);
  };
  g_skill.end = []() {
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"2301002"; });
    CharacterStatGameInstance::Update();
  };
  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    g_skill.end();
    g_skill.lv = static_cast<uint8_t>(std::clamp(ski_lv, 1, 20));
    const uint64_t duration =
        static_cast<uint64_t>(
            std::max(1, SkillInt(u"2301002", g_skill.lv, u"time", 100))) *
        1000;
    g_skill.duration = duration;
    g_skill.destroy = Window::dt_now_ + duration;
    SkillGameInstance::ski_.push_back(g_skill);
    CharacterStatGameInstance::Update();
    SendBuffState(2301002, 1);
    if (client_id == 0) {
      auto& sf = CharacterGameInstance::self_;
      auto action_type = CharacterLogicSystem::LoadActionType(sf);
      if (action_type != CharacterLogicSystem::ActionEnum::kClimb) {
        CharacterLogicSystem::RunAction(sf, u"alert2");
      } else {
        CharacterLogicSystem::self_climb_animate_cooldown_ =
            Window::dt_now_ + 700;
      }
      SendSkill(2301002, g_skill.lv, fbs::ClientCharacterAttackT{});
      SkillGameInstance::Skis()[u"2301002"].cd = Window::dt_now_ + 1000;
    }
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2301003 Bless：提升队伍命中/回避。
void Bless() {
  GameSkill g_skill;
  g_skill.id = u"2301003";
  g_skill.climb = true;
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_) &&
           JobSkillGameInstance::LoadSkiLevel(u"2301002") >= 5;
  };
  g_skill.buff_stat = [](int lv, SkillBuffStat& stat) {
    stat.acc += SkillInt(u"2301003", lv, u"indieAcc", 0);
    stat.eva += SkillInt(u"2301003", lv, u"indieEva", 0);
  };
  g_skill.end = []() {
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"2301003"; });
    CharacterStatGameInstance::Update();
  };
  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    g_skill.end();
    g_skill.lv = static_cast<uint8_t>(std::clamp(ski_lv, 1, 20));
    const uint64_t duration =
        static_cast<uint64_t>(
            std::max(1, SkillInt(u"2301003", g_skill.lv, u"time", 100))) *
        1000;
    g_skill.duration = duration;
    g_skill.destroy = Window::dt_now_ + duration;
    SkillGameInstance::ski_.push_back(g_skill);
    CharacterStatGameInstance::Update();
    SendBuffState(2301003, 1);
    if (client_id != 0) {
      return;
    }
    auto& sf = CharacterGameInstance::self_;
    auto action_type = CharacterLogicSystem::LoadActionType(sf);
    if (action_type != CharacterLogicSystem::ActionEnum::kClimb) {
      CharacterLogicSystem::RunAction(sf, u"alert2");
    } else {
      CharacterLogicSystem::self_climb_animate_cooldown_ =
          Window::dt_now_ + 700;
    }
    auto g_r = SkillGameInstance::LoadSkiR(u"2301003");
    auto players = CharacterLogicSystem::RunBuffCheck(sf, g_r);
    auto ckt =
        SkillGameInstance::CreateSkillPayload(players, 2301003, g_skill.lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, g_skill.lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
    SkillGameInstance::Skis()[u"2301003"].cd = Window::dt_now_ + 1000;
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

// 2301004 Holy Arrow：发射 3 支圣箭，最多命中 3 个目标。
void HolyArrow() {
  GameSkill g_skill;
  g_skill.id = u"2301004";
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsMagicWeapon(sf)) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"shoot1");

    const int mad = std::max(0, SkillInt(u"2301004", ski_lv, u"mad", 0));
    const int mastery =
        std::clamp(SkillInt(u"2301004", ski_lv, u"mastery", 1), 1, 10);
    const int attack_count =
        std::max(1, SkillInt(u"2301004", ski_lv, u"attackCount", 1));
    const int bullet_count =
        std::max(1, SkillInt(u"2301004", ski_lv, u"bulletCount", 3));
    // 三角形判定只选离人物最近的一个目标。
    constexpr int kTargetCount = 1;

    GameTriangle tri = {{
        SDL_FPoint{-400, -100},
        SDL_FPoint{-400, 100},
        SDL_FPoint{0, -30},
    }};
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri, kTargetCount);
    for (auto& mob : cm.data) {
      // One damage packet for all three arrows: 3 hits, 120ms apart.
      mob.hits.assign(static_cast<size_t>(attack_count * bullet_count), mad);
      mob.magic_mastery = mastery;
    }

    const auto delay = SkillGameInstance::LoadSkiTime(sf);
    const int ball_delay =
        std::max(0, SkillInt(u"2301004", ski_lv, u"ballDelay", 150));
    auto pos = sf.pos;
    pos.y -= 30;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? 400 : -400;

    uint64_t impact = delay;
    bool hit = false;
    const bool had_target = !cm.data.empty();
    bool blocked = false;
    SDL_FPoint shared_end = goal;
    for (int i = 0; i < bullet_count; ++i) {
      // All arrows share the first arrow's resolved target/terrain result.
      auto ball_cm = cm;
      SDL_FPoint arrow_goal = blocked ? shared_end : goal;
      auto cct = BallGameInstance::CreateBallPayload(
          ball_cm, pos, arrow_goal, delay + i * ball_delay, sf.page, 700,
          u"230.img/skill/2301004/ball");
      if (blocked) {
        // Terrain stopped the first arrow; later arrows stop at the same point.
        cct.payload->ball->x2 = shared_end.x;
        cct.payload->ball->y2 = shared_end.y;
      }
      ClientRequest::SendToHost(cct);
      ServerBallInstance::HandleServerB(cct.payload);
      if (i == 0) {
        cm = std::move(ball_cm);
        impact = BallGameInstance::LoadBallTime(cct);
        hit = !cm.data.empty();
        if (had_target && !hit) {
          blocked = true;
          shared_end = {cct.payload->ball->x2, cct.payload->ball->y2};
        }
      }
    }

    fbs::ClientCharacterAttackT cat;
    if (hit) {
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, impact, 120,
                                                  "", "");
      ClientRequest::SendToHost(cat);
    }
    SendSkill(2301004, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

}  // namespace

[[maybe_unused]] static const bool r = [] {
  MpEaterPassive();
  Teleport();
  Heal();
  Invincible();
  Bless();
  HolyArrow();
  return true;
}();

}  // namespace sdlms
