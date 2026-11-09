#include "src/client/game/game_effect.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_skill/mp_eater.h"
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
#include "src/client/game_instance/random_game_instance.h"
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
#include "src/server/server_instance/server_mob_instance.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <any>
#include <array>
#include <cstdint>
#include <flat_map>
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

void ApplyMobDebuff(GameDebuff::DebuffType type, CheckMobs& cm,
                    uint64_t delay, uint64_t duration, int64_t sub_val = 0,
                    int64_t dot_damage = 0, int64_t dot_interval = 1000) {
  fbs::ClientMobDebuffT cmb;
  cmb.map_id = SceneSystemInstance::map_id_;
  for (auto& mob : cm.data) {
    auto it = MobGameInstance::data_.find(mob.mob.index);
    if (it == MobGameInstance::data_.end()) {
      continue;
    }
    if (it->second.mob.debuffs.contains(type)) {
      continue;
    }
    auto md = std::make_unique<fbs::MobDebuffT>();
    md->mob_index = mob.mob.index;
    md->payload = std::make_unique<fbs::StateT>();
    md->payload->val = static_cast<int64_t>(type);
    if (type == GameDebuff::DebuffType::kBurn ||
        type == GameDebuff::DebuffType::kPoison) {
      // DOT：低 16 位为每跳伤害（基础魔法攻击 + WZ dot），高 16 位为间隔。
      md->payload->sub_val =
          (dot_damage & 0xFFFF) | ((dot_interval & 0xFFFF) << 16);
    } else {
      md->payload->sub_val = sub_val;
    }
    md->payload->delay = delay;
    md->payload->time = delay + duration;
    cmb.payload.push_back(std::move(md));
  }
  if (!cmb.payload.empty()) {
    ServerMobInstance::HandleDebuff(0, cmb);
  }
}

// MP Eater：攻击魔法命中后 5 秒一次，按 WZ prop 触发，吸取目标 MP。
void TryMpEater(CheckMobs& cm, int ski_lv) {
  static uint64_t next = 0;
  if (Window::dt_now_ < next || cm.data.empty() || ski_lv <= 0) {
    return;
  }
  const auto& sf = CharacterGameInstance::self_;
  if (sf.job != u"210") {
    return;
  }
  const int prop = SkillInt(u"2100000", ski_lv, u"prop", 0);
  std::uniform_int_distribution<int> roll(0, 99);
  if (roll(RandomGameInstance::gen_) >= prop) {
    return;
  }
  auto& target = cm.data[0];
  auto it = MobGameInstance::data_.find(target.mob.index);
  if (it == MobGameInstance::data_.end() || it->second.mob.mp <= 0) {
    return;
  }
  const int x = std::max(0, SkillInt(u"2100000", ski_lv, u"x", 0));
  const int64_t drain = std::max<int64_t>(
      1, it->second.mob.mp * x / 100);
  it->second.mob.mp = std::max<int64_t>(0, it->second.mob.mp - drain);
  target.mob.mp = it->second.mob.mp;
  CharacterStatGameInstance::SetMp(std::min(
      CharacterStatGameInstance::GetMpMax(),
      CharacterStatGameInstance::GetMp() + static_cast<int32_t>(drain)));
  next = Window::dt_now_ + 5000;

  // 通过技能包播放 2100000/effect，服务端会同步给同地图其他玩家。
  SendSkill(2100000, std::max(1, ski_lv),
            fbs::ClientCharacterAttackT{});
}

void MpEaterPassive() {
  GameSkill g_skill;
  g_skill.id = u"2100000";
  g_skill.passive = [](int) {};
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void FireArrow() {
  GameSkill g_skill;
  g_skill.id = u"2101003";
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsMagicWeapon(sf)) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"shoot1");

    const int mad = std::max(0, SkillInt(u"2101003", ski_lv, u"mad", 0));
    const int mastery =
        std::clamp(SkillInt(u"2101003", ski_lv, u"mastery", 1), 1, 10);
    const int dot = std::max(0, SkillInt(u"2101003", ski_lv, u"dot", 0));
    const int dot_time =
        std::max(0, SkillInt(u"2101003", ski_lv, u"dotTime", 0));
    const int dot_damage =
        dot > 0 && dot_time > 0
            ? CharacterStatGameInstance::GetMagic() + dot
            : 0;

    GameTriangle tri = {{
        SDL_FPoint{-400, -100},
        SDL_FPoint{-400, 100},
        SDL_FPoint{0, -30},
    }};
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri, 1);
    for (auto& mob : cm.data) {
      mob.hits = {mad};
      mob.magic_mastery = mastery;
    }

    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto pos = sf.pos;
    pos.y -= 30;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? 400 : -400;
    auto cct = BallGameInstance::CreateBallPayload(
        cm, pos, goal, delay, sf.page, 700, u"210.img/skill/2101003/ball");
    ClientRequest::SendToHost(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d);
      ClientRequest::SendToHost(cat);
      ApplyMobDebuff(GameDebuff::DebuffType::kBurn, cm, d,
                     static_cast<uint64_t>(dot_time) * 1000, 0, dot_damage,
                     1000);
    }
    SendSkill(2101003, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void PoisonBreath() {
  GameSkill g_skill;
  g_skill.id = u"2101004";
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_);
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsMagicWeapon(sf)) {
      return;
    }
    const std::array actions = {u"swingO1", u"swingO2", u"swingO3"};
    std::uniform_int_distribution<> dis(0, actions.size() - 1);
    CharacterLogicSystem::RunAction(sf,
                                    actions[dis(RandomGameInstance::gen_)]);

    const int mad = std::max(0, SkillInt(u"2101005", ski_lv, u"mad", 0));
    const int mastery =
        std::clamp(SkillInt(u"2101005", ski_lv, u"mastery", 1), 1, 10);
    const int dot = std::max(0, SkillInt(u"2101005", ski_lv, u"dot", 0));
    const int dot_time =
        std::max(0, SkillInt(u"2101005", ski_lv, u"dotTime", 0));
    const int dot_damage =
        dot > 0 && dot_time > 0
            ? CharacterStatGameInstance::GetMagic() + dot
            : 0;
    const int mob_count =
        std::max(1, SkillInt(u"2101005", ski_lv, u"mobCount", 4));

    auto* hidden_level =
        SkillGameInstance::LoadSkiLevelNode(u"2101005", ski_lv);
    if (hidden_level == nullptr) {
      return;
    }
    auto lt = WzResource::LoadFpoint(hidden_level->get_child(u"lt"));
    auto rb = WzResource::LoadFpoint(hidden_level->get_child(u"rb"));
    GameTriangle tri = {{
        SDL_FPoint{-400, -100},
        SDL_FPoint{-400, 100},
        SDL_FPoint{0, -30},
    }};
    auto hit_cm = CharacterLogicSystem::RunAttackCheck(sf, tri, 1);

    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto pos = sf.pos;
    pos.y -= 30;
    SDL_FPoint goal = pos;
    goal.x += sf.flip ? 400 : -400;
    auto cct = BallGameInstance::CreateBallPayload(
        hit_cm, pos, goal, delay, sf.page, 700,
        u"210.img/skill/2101004/ball");
    ClientRequest::SendToHost(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    auto impact = BallGameInstance::LoadBallTime(cct);
    fbs::ClientCharacterAttackT cat;
    if (!hit_cm.data.empty()) {
      const auto impact_pos = hit_cm.data[0].mob.pos;
      auto area_rect =
          SkillGameInstance::LoadR(lt, rb, impact_pos, sf.flip);
      auto cm =
          CharacterLogicSystem::RunAttackCheck(impact_pos, area_rect, mob_count);
      for (auto& mob : cm.data) {
        mob.hits = {mad};
        mob.magic_mastery = mastery;
      }
      if (!cm.data.empty()) {
        cat = SkillGameInstance::CreateAttackPayload(cm, impact_pos, impact);
        ClientRequest::SendToHost(cat);
        const int prop =
            std::clamp(SkillInt(u"2101005", ski_lv, u"prop", 0), 0, 100);
        std::uniform_int_distribution<int> roll(0, 99);
        if (roll(RandomGameInstance::gen_) < prop) {
          ApplyMobDebuff(GameDebuff::DebuffType::kPoison, cm, impact,
                         static_cast<uint64_t>(dot_time) * 1000, 0,
                         dot_damage, 1000);
        }
      }
    }
    SendSkill(2101004, ski_lv, cat);
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void Meditation() {
  GameSkill g_skill;
  g_skill.id = u"2101000";
  g_skill.climb = true;
  g_skill.can_use = [](int) {
    return JobSkillGameInstance::LoadSkiLevel(u"2100000") >= 3;
  };
  g_skill.buff_stat = [](int lv, SkillBuffStat& stat) {
    stat.mad += SkillInt(u"2101000", lv, u"indieMad", 0);
  };
  g_skill.end = []() {
    std::erase_if(SkillGameInstance::ski_,
                  [](const GameSkill& s) { return s.id == u"2101000"; });
    CharacterStatGameInstance::Update();
  };
  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    g_skill.end();
    g_skill.lv = static_cast<uint8_t>(std::clamp(ski_lv, 1, 20));
    const auto duration =
        static_cast<uint64_t>(std::max(
            1, SkillInt(u"2101000", g_skill.lv, u"time", 1))) *
        1000;
    g_skill.destroy = Window::dt_now_ + duration;
    SkillGameInstance::ski_.push_back(g_skill);
    CharacterStatGameInstance::Update();
    SendBuffState(2101000, 1);
    if (client_id != 0) {
      return;
    }
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAction(sf, u"alert2");
    auto g_r = SkillGameInstance::LoadSkiR(u"2101000");
    auto players = CharacterLogicSystem::RunBuffCheck(sf, g_r);
    auto ckt =
        SkillGameInstance::CreateSkillPayload(players, 2101000, g_skill.lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, g_skill.lv, ckt.payload, 0);
    ClientRequest::SendToHost(ckt);
    SkillGameInstance::Skis()[u"2101000"].cd = Window::dt_now_ + 1000;
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void Teleport() {
  GameSkill g_skill;
  g_skill.id = u"2101001";
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
    e->id = u"2101001";
    return true;
  };
  g_skill.use = [g_skill](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsMagicWeapon(sf)) {
      return;
    }
    SkillGameInstance::Skis()[u"2101001"].cd = Window::dt_now_ + 500;
    const int range = std::max(
        1, SkillInt(u"2101001", ski_lv, u"range", 110));
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
    SendSkill(2101001, ski_lv, fbs::ClientCharacterAttackT{});
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

void Slow() {
  GameSkill g_skill;
  g_skill.id = u"2101002";
  g_skill.can_use = [](int) {
    return IsMagicWeapon(CharacterGameInstance::self_) &&
           JobSkillGameInstance::LoadSkiLevel(u"2101001") >= 5;
  };
  g_skill.use = [](uint64_t, int ski_lv) {
    auto& sf = CharacterGameInstance::self_;
    if (!IsMagicWeapon(sf)) {
      return;
    }
    CharacterLogicSystem::RunAction(sf, u"alert2");
    const int mob_count =
        std::max(1, SkillInt(u"2101002", ski_lv, u"mobCount", 6));
    const int speed =
        std::max(0, SkillInt(u"2101002", ski_lv, u"speed", 0));
    const int time_sec =
        std::max(1, SkillInt(u"2101002", ski_lv, u"time", 10));
    auto rect = SkillGameInstance::LoadSkiR(u"2101002");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, rect, mob_count);
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    ApplyMobDebuff(GameDebuff::DebuffType::kSlow, cm, delay,
                   static_cast<uint64_t>(time_sec) * 1000, speed);
    SendSkill(2101002, ski_lv, fbs::ClientCharacterAttackT{});
  };
  SkillGameInstance::Skis()[g_skill.id] = g_skill;
}

}  // namespace

void TriggerMpEater(CheckMobs& cm) {
  const int lv = JobSkillGameInstance::LoadSkiLevel(u"2100000");
  if (lv > 0) {
    TryMpEater(cm, lv);
  }
}

[[maybe_unused]] static const bool r = [] {
  MpEaterPassive();
  Meditation();
  Teleport();
  Slow();
  FireArrow();
  PoisonBreath();
  return true;
}();

}  // namespace sdlms
