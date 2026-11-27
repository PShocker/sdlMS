#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/craft_ui_system.h"

#include "src/client/game/game_character.h"
#include "src/client/game/game_tomb.h"
#include "src/client/game_skill/crossbow_attack.h"
#include "src/client/game_skill/fighter_final_attack.h"
#include "src/client/game_skill/hunter_attack.h"
#include "src/client/game_skill/page_final_attack.h"
#include "src/client/game_skill/spearman_final_attack.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/ladderrope_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/party_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/reactor_game_instance.h"
#include "src/client/game_instance/seat_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/game_instance/triangle_game_instance.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system/ui/revive_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/script/script.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_main.h"

#include <SDL3/SDL_rect.h>
#include <wz/Property.h>

#include <algorithm>
#include <any>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <flat_map>
#include <flat_set>
#include <format>
#include <memory>
#include <numbers>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {

namespace {

int32_t LoadMobInfoInt(wz::Node* node, const char16_t* key, int32_t def) {
  if (node == nullptr) {
    return def;
  }
  auto* child = node->get_child(key);
  if (child == nullptr || child->type != wz::Type::Int) {
    return def;
  }
  return static_cast<wz::Property<int32_t>*>(child)->get();
}

}  // namespace

static const std::flat_set<EquipGameInstance::WeaponType> shoot_weapons = {
    EquipGameInstance::WeaponType::kBow,
    EquipGameInstance::WeaponType::kCrossbow,
    EquipGameInstance::WeaponType::kClaw,
    EquipGameInstance::WeaponType::kGun,
};
static const std::flat_map<EquipGameInstance::WeaponType,
                           std::flat_set<std::u16string>>
    weapon_attack_action = {
        {EquipGameInstance::WeaponType::kS1A1M1D,
         {u"stabO1", u"stabO2", u"swingO1", u"swingO2", u"swingO3"}},
        {EquipGameInstance::WeaponType::kSpear, {u"stabT1", u"swingP1"}},
        {EquipGameInstance::WeaponType::kBow, {u"shoot1"}},
        {EquipGameInstance::WeaponType::kCrossbow, {u"shoot2"}},
        {EquipGameInstance::WeaponType::kS2A2M2,
         {u"stabO1", u"stabO2", u"swingT1", u"swingT2", u"swingT3"}},
        {EquipGameInstance::WeaponType::kWand, {u"swingO1", u"swingO2"}},
        {EquipGameInstance::WeaponType::kClaw, {u"swingO1", u"swingO2"}},
        {EquipGameInstance::WeaponType::kGun, {u"shot"}},
};
static const std::flat_map<EquipGameInstance::WeaponType,
                           std::flat_set<std::u16string>>
    weapon_attack_action2 = {
        {EquipGameInstance::WeaponType::kBow, {u"swingT1", u"swingT3"}},
        {EquipGameInstance::WeaponType::kCrossbow, {u"swingT1", u"stabT1"}},
        {EquipGameInstance::WeaponType::kClaw, {u"stabO1", u"stabO2"}},
        {EquipGameInstance::WeaponType::kGun, {u"swingP1", u"stabT2"}},
};

SDL_FRect CharacterLogicSystem::LoadRect(GameCharacter& g_character) {
  const auto stand_w = 44.0f;
  const auto stand_h = 65.0f;
  const auto prone_w = 46.0f;
  const auto prone_h = 31.0f;
  SDL_FRect rect;
  if (g_character.action == u"prone" || g_character.action == u"proneStab") {
    rect.x = g_character.pos.x - prone_w / 2;
    rect.y = g_character.pos.y - prone_h;
    rect.w = prone_w;
    rect.h = prone_h;
  } else {
    rect.x = g_character.pos.x - stand_w / 2;
    rect.y = g_character.pos.y - stand_h;
    rect.w = stand_w;
    rect.h = stand_h;
  }
  if (g_character.flip == 1) {
    rect.x += 2 * (g_character.pos.x - rect.x) - rect.w;
  }
  return rect;
}

CheckReactors CharacterLogicSystem::RunReactorCheck(GameCharacter& g,
                                                    SDL_FRect g_r) {
  auto& g_pos = g.pos;
  std::vector<CheckReactors::Reactors> v;
  std::flat_map<uint32_t, CheckReactors::Reactors> m;
  for (const auto& rs : ReactorGameInstance::data_) {
    for (const auto& r : rs) {
      auto node = WzResource::reactor_->find(r.id + u".img");
      node = node->find(std::to_string(r.state) + "/event/0");
      if (node == nullptr) {
        continue;
      }
      auto type =
          static_cast<wz::Property<int>*>(node->get_child(u"type"))->get();
      if (type != 0) {
        continue;
      }
      SDL_FRect reactor_r;
      reactor_r.x = r.pos.x - 20;
      reactor_r.y = r.pos.y - 20;
      reactor_r.w = 40;
      reactor_r.h = 40;
      if (SDL_HasRectIntersectionFloat(&reactor_r, &g_r)) {
        auto& r_pos = r.pos;
        auto dis = (r_pos.x - g_pos.x) * (r_pos.x - g_pos.x) +
                   (r_pos.y - g_pos.y) * (r_pos.y - g_pos.y);
        SDL_FRect res;
        SDL_GetRectIntersectionFloat(&reactor_r, &g_r, &res);
        float attack_x = res.x + res.w / 2 - r_pos.x;
        float attack_y = res.y + res.h / 2 - r_pos.y;
        m[dis] = {
            .r = r,
            .x = attack_x,
            .y = attack_y,
        };
      }
    }
  }
  v.append_range(m.values());
  return {v};
}

CheckMobs CharacterLogicSystem::RunAttackCheck(GameCharacter& g_character,
                                               GameTriangle tri, int num) {
  std::vector<CheckMobs::Mobs> v;
  std::vector<CheckMobs::Mobs> miss_v;
  std::flat_multimap<float, CheckMobs::Mobs> m;
  std::flat_multimap<float, CheckMobs::Mobs> miss_m;
  auto& g_pos = g_character.pos;
  auto t =
      TriangleGameInstance::LoadTri(tri, g_character.flip, g_character.pos);
  for (const auto& [k, v] : MobGameInstance::data_) {
    auto& mob = v.mob;
    // 渐显中的怪物不参与人物攻击判定。
    if (mob.revive_time != 0 && Window::dt_now_ - mob.revive_time < 1000) {
      continue;
    }
    if (mob.hp <= 0) {
      continue;
    }
    auto mob_action = MobLogicSystem::LoadActionType(mob.action);
    if (mob_action == MobLogicSystem::ActionEnum::kRevive ||
        mob_action == MobLogicSystem::ActionEnum::kDie) {
      continue;
    }
    auto* info = MobGameInstance::LoadMobInfo(mob.id);
    auto chance = CharacterStatGameInstance::CalcHitChance(
        static_cast<uint32_t>(LoadMobInfoInt(info, u"eva", 0)),
        static_cast<uint32_t>(LoadMobInfoInt(info, u"level", 1)));
    std::uniform_int_distribution<uint32_t> hit_roll(0, 99);
    auto m_r_opt = MobLogicSystem::LoadRect(mob);
    if (!m_r_opt.has_value()) {
      continue;
    }
    auto& m_r = m_r_opt.value();

    if (TriangleGameInstance::RectInsTri(m_r, t)) {
      auto& m_pos = mob.pos;
      auto dis = (m_pos.x - g_pos.x) * (m_pos.x - g_pos.x) +
                 (m_pos.y - g_pos.y) * (m_pos.y - g_pos.y);
      CheckMobs::Mobs cm = {
          .mob = v.mob,
          .x = 0,
          .y = 0,
      };
      if (hit_roll(RandomGameInstance::gen_) >= chance) {
        miss_m.emplace(dis, cm);
      } else {
        m.emplace(dis, cm);
      }
    }
  }
  if (num == 1) {
    const bool hit_closer =
        !m.empty() && (miss_m.empty() || m.begin()->first <= miss_m.begin()->first);
    if (hit_closer) {
      miss_m.clear();
    } else {
      m.clear();
    }
  }
  if (m.size() > num) {
    m.erase(m.begin() + num, m.end());
  }
  if (miss_m.size() > num) {
    miss_m.erase(miss_m.begin() + num, miss_m.end());
  }
  v.append_range(m.values());
  miss_v.append_range(miss_m.values());
  return {v, miss_v};
}

CheckMobs CharacterLogicSystem::RunAttackCheck(GameCharacter& g_character,
                                               GameTriangle tri) {
  return RunAttackCheck(g_character, tri, 1);
}

CheckMobs CharacterLogicSystem::RunAttackCheck(SDL_FPoint pos, SDL_FRect g_r,
                                               int num) {
  std::vector<CheckMobs::Mobs> v;
  std::vector<CheckMobs::Mobs> miss_v;
  std::flat_multimap<float, CheckMobs::Mobs> m;
  std::flat_multimap<float, CheckMobs::Mobs> miss_m;
  auto& g_pos = pos;
  for (const auto& [k, v] : MobGameInstance::data_) {
    auto& mob = v.mob;
    // 渐显中的怪物不参与人物攻击判定。
    if (mob.revive_time != 0 && Window::dt_now_ - mob.revive_time < 1000) {
      continue;
    }
    auto mob_action = MobLogicSystem::LoadActionType(mob.action);
    if (mob_action == MobLogicSystem::ActionEnum::kRevive ||
        mob_action == MobLogicSystem::ActionEnum::kDie) {
      continue;
    }
    if (mob.hp <= 0) {
      continue;
    }
    auto* info = MobGameInstance::LoadMobInfo(mob.id);
    auto chance = CharacterStatGameInstance::CalcHitChance(
        static_cast<uint32_t>(LoadMobInfoInt(info, u"eva", 0)),
        static_cast<uint32_t>(LoadMobInfoInt(info, u"level", 1)));
    std::uniform_int_distribution<uint32_t> hit_roll(0, 99);
    auto m_r_opt = MobLogicSystem::LoadRect(mob);
    if (!m_r_opt.has_value()) {
      continue;
    }
    auto& m_r = m_r_opt.value();
    if (SDL_HasRectIntersectionFloat(&m_r, &g_r)) {
      auto& m_pos = mob.pos;
      auto dis = (m_pos.x - g_pos.x) * (m_pos.x - g_pos.x) +
                 (m_pos.y - g_pos.y) * (m_pos.y - g_pos.y);
      SDL_FRect res;
      SDL_GetRectIntersectionFloat(&m_r, &g_r, &res);
      float attack_x = res.x + res.w / 2 - m_pos.x;
      float attack_y = res.y + res.h / 2 - m_pos.y;
      CheckMobs::Mobs cm = {
          .mob = v.mob,
          .x = attack_x,
          .y = attack_y,
      };
      if (hit_roll(RandomGameInstance::gen_) >= chance) {
        miss_m.emplace(dis, cm);
      } else {
        m.emplace(dis, cm);
      }
    }
  }
  if (num == 1) {
    const bool hit_closer =
        !m.empty() && (miss_m.empty() || m.begin()->first <= miss_m.begin()->first);
    if (hit_closer) {
      miss_m.clear();
    } else {
      m.clear();
    }
  }
  if (m.size() > num) {
    m.erase(m.begin() + num, m.end());
  }
  if (miss_m.size() > num) {
    miss_m.erase(miss_m.begin() + num, miss_m.end());
  }
  v.append_range(m.values());
  miss_v.append_range(miss_m.values());
  return {v, miss_v};
}

CheckMobs CharacterLogicSystem::RunAttackCheck(GameCharacter& g_character,
                                               SDL_FRect g_r, int num) {
  return RunAttackCheck(g_character.pos, g_r, num);
}

CheckMobs CharacterLogicSystem::RunAttackCheck(GameCharacter& g_character,
                                               SDL_FRect g_r) {
  return RunAttackCheck(g_character, g_r, UINT32_MAX);
}

bool CharacterLogicSystem::RunAction(GameCharacter& g_character,
                                     const std::u16string& action) {
  if (g_character.action == action) {
    return false;
  }
  g_character.action = action;
  g_character.action_index = 0;
  g_character.action_time = 0;
  return true;
}

bool CharacterLogicSystem::RunFaceAction(GameCharacter& g_character,
                                         const std::u16string& action) {
  if (g_character.face.id == action) {
    return false;
  }
  g_character.face.action = action;
  g_character.face.index = 0;
  g_character.face.time = 0;
  g_character.face.destroy = Window::dt_now_ + 4000;
  return true;
}

void CharacterLogicSystem::RunWalkAction(GameCharacter& g_character) {
  if (g_character.weapon.has_value()) {
    auto weapon_id = g_character.weapon.value().id;
    const auto& avatar_data = CharacterGameInstance::avatar_data_;
    if (avatar_data.at(weapon_id).data.contains(u"walk1")) {
      RunAction(g_character, u"walk1");
    } else {
      RunAction(g_character, u"walk2");
    }
  } else {
    RunAction(g_character, u"walk1");
  }
  return;
}

void CharacterLogicSystem::RunStandAction(GameCharacter& g_character) {
  if (g_character.weapon.has_value()) {
    auto weapon_id = g_character.weapon.value().id;
    const auto& avatar_data = CharacterGameInstance::avatar_data_;
    if (avatar_data.at(weapon_id).data.contains(u"stand1")) {
      RunAction(g_character, u"stand1");
    } else {
      RunAction(g_character, u"stand2");
    }
  } else {
    RunAction(g_character, u"stand1");
  }
  return;
}

void CharacterLogicSystem::RunClimbAction(GameCharacter& g_character) {
  const auto& c_lr = LadderropeGameInstance::data_.at(self_lr_);
  if (c_lr.l == 1) {
    RunAction(g_character, u"ladder");
  } else {
    RunAction(g_character, u"rope");
  }
  return;
}

bool CharacterLogicSystem::RunAnimate(GameCharacter& g_character) {
  bool r = false;
  if (!g_character.action_animate) {
    return r;
  }
  auto delta = Window::delta_time_;
  auto action_type = LoadActionType(g_character);
  float speed = 1;
  switch (action_type) {
    case ActionEnum::kAttack:
    case ActionEnum::kSkill: {
      speed = LoadAttackSpeed(g_character);
      break;
    }
    default: {
      break;
    }
  }
  g_character.action_time += delta * speed;
  //   获取动作延迟时间
  uint32_t delay = 100;
  uint32_t size = 0;
  if (!g_character.morph.empty()) {
    auto node = WzResource::morph_->find(g_character.morph + u".img");
    auto action = LoadMorphType(g_character);
    node = node->get_child(action);
    size = node->children_count();
    node = node->get_child(std::to_string(g_character.action_index));
    if (node->get_child(u"delay")) {
      delay = static_cast<wz::Property<int>*>(node->get_child(u"delay"))->get();
    }
  } else if (CharacterGameInstance::extern_action_.contains(
                 g_character.action)) {
    auto& action_info =
        CharacterGameInstance::extern_action_.at(g_character.action);
    delay = std::abs(action_info[g_character.action_index].delay);
    size = action_info.size();
  } else {
    auto& action_info =
        CharacterGameInstance::bone_data_.at(g_character.action);
    delay = action_info[g_character.action_index].delay;
    size = action_info.size();
  }
  if (g_character.action_time >= delay) {
    g_character.action_index += 1;
    r = g_character.action_index >= size;
    g_character.action_index = g_character.action_index % size;
    g_character.action_time = 0;
  }
  return r;
}

bool CharacterLogicSystem::RunFlip(GameCharacter& g_character) {
  // 在梯子/绳子上固定朝左，方向键不能改变朝向。
  if (self_lr_ != 0) {
    g_character.flip = 0;
    return false;
  }
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return false;
  }
  // 095 nFreeze：冰冻期间不能转向。
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kFreeze)) {
    return false;
  }
  if (self_flip_cooldown_ > Window::dt_now_) {
    return false;
  }
  if (character_action_input_.contains("left")) {
    g_character.flip = 0;
    self_flip_cooldown_ = Window::dt_now_ + 50;
    return true;
  }
  if (character_action_input_.contains("right")) {
    g_character.flip = 1;
    self_flip_cooldown_ = Window::dt_now_ + 50;
    return true;
  }
  return false;
}

void CharacterLogicSystem::RunPick(GameCharacter& g_character) {
  if (ccp_.random_id != 0) {
    // 服务器没有回包/掉落物已被别人捡走时避免永久卡住。
    if (Window::dt_now_ - ccp_time_ > 500) {
      ccp_ = {};
      ccp_time_ = 0;
    } else {
      return;
    }
  }
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return;
  }
  if (character_action_input_.contains("pick")) {
    for (auto& v : DropGameInstance::data_ | std::views::values) {
      if (v.type == GameDrop::DropEnum::kLand) {
        if (PackageGameInstance::LoadEmptyIndex(v.data).empty()) {
          continue;
        }
        auto pos = g_character.pos;
        if (pos.x == std::clamp(pos.x, v.goal.x - 20, v.goal.x + 40) &&
            pos.y == std::clamp(pos.y, v.goal.y - 20, v.goal.y + 20)) {
          ccp_.map_id = SceneSystemInstance::map_id_;
          ccp_.random_id = v.random_id;
          // pet_index < 0 表示由角色自己捡（宠物捡取时填宠物栏位）。
          ccp_.pet_index = -1;
          ccp_time_ = Window::dt_now_;
          ClientRequest::SendToHost(ccp_);
          return;
        }
      }
    }
    return;
  }
}

bool CharacterLogicSystem::RunHarvest(GameCharacter& g_character) {
  bool r = false;
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return r;
  }
  if (character_action_input_.contains("harvest")) {
    if (CursorGameInstance::cursor_hand_net_.has_value()) {
      return false;
    }
    // npc talk
    auto g_pos = g_character.pos;
    for (auto& m : NpcGameInstance::data_) {
      for (auto& g_npc : m) {
        auto n_pos = g_npc.pos;
        auto dx = n_pos.x - g_pos.x;
        auto dy = n_pos.y - g_pos.y;
        if (dx * dx + dy * dy <= std::pow(100, 2)) {
          return CursorLogicSystem::EventClickNpc(g_npc);
        }
      }
    }
  }
  return r;
}

bool CharacterLogicSystem::RunWalk(GameCharacter& g_character) {
  bool r = true;
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return r;
  }
  // 095 SecondaryStat nFreeze：冰冻期间不能移动（攻击/技能不受限）。
  const bool frozen =
      g_character.debuffs.contains(GameDebuff::DebuffType::kFreeze);
  // 095 SecondaryStat nSlow：移动速度按百分比下降。
  float slow = 1.0f;
  if (auto it = g_character.debuffs.find(GameDebuff::DebuffType::kSlow);
      it != g_character.debuffs.end()) {
    slow = std::clamp(1.0f - static_cast<float>(it->second.val) / 100.0f, 0.1f,
                      1.0f);
  }
  // 095 SecondaryStat nDisorder：左右输入反转。
  const bool reverse =
      g_character.debuffs.contains(GameDebuff::DebuffType::kDisorder);

  // 095 CAttrFoothold：当前落脚点的 walk/drag/force 影响行走。
  float walk = 1.0f;
  float drag = 1.0f;
  float force = 0.0f;
  if (self_fh_ != 0) {
    auto it = FootholdGameInstance::data_.find(self_fh_);
    if (it != FootholdGameInstance::data_.end()) {
      walk = it->second.walk;
      drag = it->second.drag;
      force = it->second.force;
    }
  }

  bool input_left = character_action_input_.contains("left");
  bool input_right = character_action_input_.contains("right");
  if (reverse) {
    std::swap(input_left, input_right);
  }
  if (frozen) {
    input_left = false;
    input_right = false;
  }

  if (input_left) {
    self_hforce_ = -1400;
    RunWalkAction(g_character);
  } else if (input_right) {
    self_hforce_ = 1400;
    RunWalkAction(g_character);
  } else {
    // 左右都没输入，stand或alert
    if (self_alert_cooldown_ > Window::dt_now_) {
      RunAction(g_character, u"alert");
    } else {
      RunStandAction(g_character);
    }
    self_hforce_ = 0;
    if (self_hspeed_ == 0 && force == 0) {
      return r;
    }
  }
  // 移动
  auto delta_time = Window::delta_time_ / 1000.0f;

  auto map_id = SceneSystemInstance::map_id_;
  auto border = MapInfoGameInstance::LoadMrBorder(map_id);

  auto hforce = self_hforce_;
  if (drag != 1.0f) {
    hforce *= drag;
  }
  auto hspeed_min = CharacterStatGameInstance::GetHspeedMin();
  auto hspeed_max = CharacterStatGameInstance::GetHspeedMax();
  if (walk != 1.0f) {
    hspeed_min *= walk;
    hspeed_max *= walk;
  }
  // 095 nSlow：速度上限按百分比下降（下限保留 10%）。
  if (slow != 1.0f) {
    hspeed_min *= slow;
    hspeed_max *= slow;
  }
  if (force != 0.0f) {
    // 095 CVecCtrl::CalcWalk：force 为传送带恒力（WZ 百分数 /100）。
    auto input = input_left ? -1 : input_right ? 1 : 0;
    auto magnitude = std::abs(force);
    if (input == 0) {
      hforce += force * 1400.0f;
    } else if (input * force > 0) {
      hforce *= std::clamp(2.0f * magnitude, 0.1f, 4.0f);
    } else {
      hforce *= std::clamp(0.2f / magnitude, 0.1f, 4.0f);
    }
  }

  r = Physic::Walk(g_character.pos, delta_time, self_hspeed_, self_vspeed_,
                   hforce, hspeed_min, hspeed_max, 800, true, self_fh_,
                   border, FootholdGameInstance::data_);
  return r;
}

bool CharacterLogicSystem::RunFall(GameCharacter& g_character) {
  if (self_fh_ != 0) {
    return false;
  }
  // 095 nFreeze：冰冻期间不能改变水平速度（相当于定身在空中）。
  const bool frozen =
      g_character.debuffs.contains(GameDebuff::DebuffType::kFreeze);
  if (!frozen &&
      !g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    if (character_action_input_.contains("left")) {
      self_hspeed_ -= 0.1 * Window::delta_time_;
    } else if (character_action_input_.contains("right")) {
      self_hspeed_ += 0.1 * Window::delta_time_;
    }
  }
  auto delta_time = Window::delta_time_ / 1000.0;
  auto vspeed = self_vspeed_ + delta_time * 2000;
  if (self_vspeed_ <= 0 && vspeed > 0) {
    self_fall_min_ = g_character.pos.y;
  }
  self_vspeed_ = vspeed;
  auto fall_collide = self_foothold_cooldown_ <= Window::dt_now_;

  auto map_id = SceneSystemInstance::map_id_;
  auto border = MapInfoGameInstance::LoadMrBorder(map_id);

  bool r = Physic::Fall(g_character.pos, delta_time, self_hspeed_, self_vspeed_,
                        CharacterStatGameInstance::GetVspeedMin(),
                        CharacterStatGameInstance::GetVspeedMax(), border,
                        fall_collide, true, self_fh_, g_character.page,
                        FootholdGameInstance::data_, self_fall_fh_);
  if (!r) {
    self_two_jump_cooldown_ = 0;
    self_fall_fh_ = self_fh_;
  }
  return r;
}

bool CharacterLogicSystem::RunProne(GameCharacter& g_character) {
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return false;
  }
  if (self_fh_ == 0 || self_lr_ != 0) {
    return false;
  }
  if (character_action_input_.contains("down")) {
    RunAction(g_character, u"prone");
    self_hspeed_ = 0;
    self_vspeed_ = 0;
    return true;
  }
  return false;
}

bool CharacterLogicSystem::RunJump(GameCharacter& g_character) {
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return false;
  }
  if (character_action_input_.contains("jump")) {
    auto g_action = LoadActionType(g_character);
    switch (g_action) {
      case ActionEnum::kStand:
      case ActionEnum::kAlert:
      case ActionEnum::kWalk: {
        self_vspeed_ = CharacterStatGameInstance::GetVspeedMin() * 0.1f;
        break;
      }
      case ActionEnum::kProne: {
        // 下跳
        const auto& c_fh = FootholdGameInstance::data_.at(self_fh_);
        if (!c_fh.forbid_fall_down) {
          for (auto& fh : FootholdGameInstance::data_ | std::views::values) {
            if (fh.x1 > fh.x2) {
              continue;
            }
            const auto& pos = g_character.pos;
            if (pos.x < fh.x1 || pos.x > fh.x2) {
              continue;
            }
            if (fh.k.has_value()) {
              auto y = fh.k.value() * pos.x + fh.intercept.value();
              if (y >= pos.y + 10 && y <= pos.y + 600) {
                self_vspeed_ = -100;
                self_hspeed_ = 0;
                self_fh_ = 0;
                self_foothold_cooldown_ = Window::dt_now_ + 120;
                RunAction(g_character, u"jump");
                self_two_jump_cooldown_ = Window::dt_now_ + 400;
                AudioGameInstance::PlayNow(u"Game.img/Jump");
                break;
              }
            }
          }
        }
        return false;
        break;
      }
      case ActionEnum::kJump: {
        // two jump
        if (self_two_jump_cooldown_ <= Window::dt_now_) {
          auto ski_id = JobSkillGameInstance::LoadTwoJump();
          if (!ski_id.empty()) {
            RunSkill(g_character, ski_id);
          }
        }
        return true;
        break;
      }
      case ActionEnum::kClimb: {
        // 左右键优先于上下键：正在向上/向下爬时也能主动跳离绳索/梯子。
        if (character_action_input_.contains("left") ||
            character_action_input_.contains("right")) {
          self_hspeed_ = character_action_input_.contains("left") ? -140 : 140;
          g_character.flip = self_hspeed_ < 0 ? false : true;
          g_character.action_animate = true;
          self_vspeed_ = -310;
          self_lr_ = 0;
          self_ladderrope_cooldown_ = Window::dt_now_ + 120;
        } else if (character_action_input_.contains("up") ||
                   character_action_input_.contains("down")) {
          return false;
        } else {
          return false;
        }
        break;
      }
      default: {
        break;
      }
    }
    self_two_jump_cooldown_ = Window::dt_now_ + 200;
    self_fall_fh_ = self_fh_;
    self_fh_ = 0;
    RunAction(g_character, u"jump");
    AudioGameInstance::PlayNow(u"Game.img/Jump");
    return true;
  }
  return false;
}

bool CharacterLogicSystem::RunClimb(GameCharacter& g_character) {
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return false;
  }
  if (self_ladderrope_cooldown_ > Window::dt_now_) {
    return false;
  }
  if (character_action_input_.contains("up") ||
      character_action_input_.contains("down")) {
    auto& pos = g_character.pos;
    for (const auto& [id, lr] : LadderropeGameInstance::data_) {
      // 判断x坐标是否在梯子范围内
      if (pos.x >= lr.x - 15 && pos.x <= lr.x + 15) {
        float y_min = lr.t;
        float y_max = lr.b + 5;
        if (character_action_input_.contains("up")) {
          // 按上键
          if (self_fh_ != 0) {
            // 可爬范围为[b,b+5]
            y_min = lr.b;
            y_max = lr.b + 5;
          }
        } else {
          // 按下键
          if (self_fh_ != 0) {
            y_min = lr.t - 10;
            y_max = lr.t;
          }
        }

        if (pos.y >= y_min && pos.y <= y_max) {
          self_lr_ = id;
          self_vspeed_ = 0;
          self_hspeed_ = 0;
          // 上梯/上绳的瞬间固定朝左。
          g_character.flip = 0;
          pos.x = lr.x;
          pos.y = std::clamp(pos.y, (float)lr.t, (float)lr.b);
          g_character.page = lr.page;
          RunClimbAction(g_character);
          return true;
        }
      }
    }
  }
  return false;
}

bool CharacterLogicSystem::RunClimbing(GameCharacter& g_character) {
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    RunAction(g_character, u"jump");
    return false;
  }
  if (self_climb_animate_cooldown_ >= Window::dt_now_) {
    g_character.action_animate = true;
    return true;
  } else {
    g_character.action_animate = false;
  }
  if (character_action_input_.contains("up")) {
    self_vspeed_ = -100;
  } else if (character_action_input_.contains("down")) {
    self_vspeed_ = 100;
  } else {
    self_vspeed_ = 0;
  }
  g_character.action_animate = !(self_vspeed_ == 0);
  g_character.pos.y += self_vspeed_ * Window::delta_time_ / 1000.0f;
  auto c_lr = LadderropeGameInstance::data_.at(self_lr_);
  if (g_character.pos.y < c_lr.t) {
    if (c_lr.uf == 1) {
      g_character.action_animate = true;
      g_character.pos.y = c_lr.t - 5;
      self_ladderrope_cooldown_ = Window::dt_now_ + 80;
      self_vspeed_ = 0;
      self_fh_ = 0;
      self_lr_ = 0;
      return false;
    } else {
      g_character.action_animate = false;
      g_character.pos.y = c_lr.t;
    }
  } else if (g_character.pos.y > c_lr.b) {
    g_character.pos.y = c_lr.b;
    g_character.action_animate = true;
    self_ladderrope_cooldown_ = Window::dt_now_ + 80;
    self_vspeed_ = 0;
    self_fh_ = 0;
    self_lr_ = 0;
    return false;
  }

  return true;
}

bool CharacterLogicSystem::RunSitting(GameCharacter& g_character) {
  if (!character_action_input_.empty()) {
    if (self_sit_cooldown_ <= Window::dt_now_) {
      self_sit_cooldown_ = Window::dt_now_ + 300;
      RunUnsitChair(g_character);
      return false;
    }
  }
  if (self_alert_cooldown_ > Window::dt_now_) {
    RunAction(g_character, u"alert");
    return false;
  }
  return true;
}

bool CharacterLogicSystem::IsAlert() {
  return self_alert_cooldown_ > Window::dt_now_;
}

bool CharacterLogicSystem::RunSkill(GameCharacter& g_character,
                                    const std::u16string& id) {
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return false;
  }
  // 095 CField::IsUnableToUseSkill：fieldLimit bit1 禁止使用技能。
  if (MapInfoGameInstance::IsSkillDisabled(SceneSystemInstance::map_id_)) {
    return false;
  }
  if (!g_character.morph.empty()) {
    return false;
  }
  auto& skills = SkillGameInstance::Skis();
  if (skills.contains(id)) {
    auto ski_lv = JobSkillGameInstance::LoadSkiLevel(id);
    const auto& ski = skills[id];
    if (!ski.use) {
      return false;
    }
    bool use = false;
    if (ski.cd < Window::dt_now_) {
      auto action_type = LoadActionType(g_character);
      switch (action_type) {
        case ActionEnum::kJump: {
          if (ski.fall) {
            use = true;
          }
          break;
        }
        case ActionEnum::kClimb: {
          if (ski.climb) {
            use = true;
          }
          break;
        }
        default: {
          use = true;
          break;
        }
      }
    }
    if (use) {
      if (ski.can_use && !ski.can_use(ski_lv)) {
        return false;
      }
      const bool free_use = ski.skip_mp && ski.skip_mp();
      const int mp_cost =
          free_use ? 0 : SkillGameInstance::LoadSkiMpCost(id, ski_lv);
      if (mp_cost > 0 && CharacterStatGameInstance::GetMp() < mp_cost) {
        return false;
      }
      for (auto& buff : SkillGameInstance::ski_) {
        if (buff.action) {
          buff.action(&buff, id);
        }
      }
      ski.use(0, ski_lv);
      if (mp_cost > 0) {
        CharacterStatGameInstance::SetMp(CharacterStatGameInstance::GetMp() -
                                         mp_cost);
      }
      // 095 以 Skill.wz 的 cooltime 为准；技能自身设置的动画冷却不能更短。
      const int cooldown = SkillGameInstance::LoadSkiCooldown(id, ski_lv);
      if (cooldown > 0) {
        auto& used_skill = SkillGameInstance::Skis()[id];
        const uint64_t until =
            Window::dt_now_ + static_cast<uint64_t>(cooldown) * 1000;
        used_skill.cd = std::max(used_skill.cd, until);
      }
      self_alert_cooldown_ = Window::dt_now_ + 5000;
      CharacterLogicSystem::LoadSfx(g_character);
    }
  }
  return true;
}

bool CharacterLogicSystem::RunSkill(GameCharacter& g_character) {
  if (!character_skill_input_.empty()) {
    const auto& id = *character_skill_input_.begin();
    return RunSkill(g_character, {id.begin(), id.end()});
  }
  return false;
}

bool CharacterLogicSystem::RunAttackAction(GameCharacter& g_character,
                                           bool shoot, bool force_stand) {
  if (!g_character.weapon.has_value()) {
    return false;
  }
  auto g_action = LoadActionType(g_character);
  // 三蜗牛等投掷技能在趴地时使用站立攻击动作，而不是 proneStab。
  if (force_stand && g_action == ActionEnum::kProne) {
    g_action = ActionEnum::kStand;
  }
  auto g_weapon = g_character.weapon->id;
  auto g_weapon_info = EquipGameInstance::LoadEquipInfo(g_weapon);
  auto WeaponType = EquipGameInstance::LoadWeaponType(g_character);
  switch (g_action) {
    case ActionEnum::kStand:
    case ActionEnum::kAlert:
    case ActionEnum::kWalk:
    case ActionEnum::kJump: {
      auto& gen_ = RandomGameInstance::gen_;
      auto actions = &weapon_attack_action.at(WeaponType);
      if (!shoot) {
        if (weapon_attack_action2.contains(WeaponType)) {
          actions = &weapon_attack_action2.at(WeaponType);
        }
      }
      std::uniform_int_distribution<> dis(0, actions->size() - 1);
      auto selected = *std::next(actions->begin(), dis(gen_));
      RunAction(g_character, selected);
      break;
    }
    case ActionEnum::kProne: {
      RunAction(g_character, u"proneStab");
      break;
    }
    default: {
      break;
    }
  }
  return true;
}

bool CharacterLogicSystem::RunAttack(GameCharacter& g_character) {
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return false;
  }
  if (!g_character.morph.empty()) {
    return false;
  }
  if (!g_character.weapon.has_value()) {
    return false;
  }
  if (self_attack_cooldown_ > Window::dt_now_) {
    return false;
  }
  auto g_action = LoadActionType(g_character);
  if (g_action == ActionEnum::kAttack || g_action == ActionEnum::kSkill) {
    return false;
  }
  if (!character_action_input_.contains("attack")) {
    return false;
  }
  auto g_weapon = g_character.weapon->id;
  auto g_weapon_info = EquipGameInstance::LoadEquipInfo(g_weapon);
  uint64_t delay;
  auto WeaponType = EquipGameInstance::LoadWeaponType(g_character);
  bool shoot_weapon = shoot_weapons.contains(WeaponType);
  int shoot_range = 350;
  if (WeaponType == EquipGameInstance::WeaponType::kBow ||
      WeaponType == EquipGameInstance::WeaponType::kCrossbow) {
    shoot_range +=
        static_cast<int>(CharacterStatGameInstance::GetAttackRangeBonus());
  } else if (WeaponType == EquipGameInstance::WeaponType::kClaw) {
    shoot_range +=
        static_cast<int>(CharacterStatGameInstance::GetThrowRangeBonus());
  }
  auto& gen_ = RandomGameInstance::gen_;
  const std::flat_set<std::u16string>* actions;
  if (weapon_attack_action2.contains(WeaponType)) {
    actions = &weapon_attack_action2.at(WeaponType);
  } else {
    actions = &weapon_attack_action.at(WeaponType);
  }
  std::uniform_int_distribution<> dis(0, actions->size() - 1);
  auto selected = *std::next(actions->begin(), dis(gen_));
  RunAction(g_character, selected);
  auto g_r_opt = AfterimageGameInstance::LoadRect(g_character);
  if (!g_r_opt.has_value()) {
    return false;
  }
  SDL_FRect g_r = g_r_opt.value();
  auto rt = RunReactorCheck(g_character, g_r);
  auto ball = PackageGameInstance::LoadActiveBall();
  switch (g_action) {
    case ActionEnum::kStand:
    case ActionEnum::kAlert:
    case ActionEnum::kWalk: {
      self_hspeed_ = 0;
      [[fallthrough]];
    }
    case ActionEnum::kJump: {
      if (g_action == ActionEnum::kJump &&
          (WeaponType == EquipGameInstance::WeaponType::kBow ||
           WeaponType == EquipGameInstance::WeaponType::kCrossbow)) {
        shoot_weapon = false;
      }
      if (!rt.data.empty()) {
        shoot_weapon = false;
      }
      if (ball == nullptr) {
        shoot_weapon = false;
      }
      actions = &weapon_attack_action.at(WeaponType);
      if (!shoot_weapon) {
        if (weapon_attack_action2.contains(WeaponType)) {
          actions = &weapon_attack_action2.at(WeaponType);
        }
      }
      std::uniform_int_distribution<> dis(0, actions->size() - 1);
      auto selected = *std::next(actions->begin(), dis(gen_));
      RunAction(g_character, selected);
      break;
    }
    case ActionEnum::kProne: {
      shoot_weapon = false;
      RunAction(g_character, u"proneStab");
      break;
    }
    default: {
      return false;
      break;
    }
  }
  delay = AfterimageGameInstance::LoadBeatTime(g_character);
  self_alert_cooldown_ = Window::dt_now_ + 5000;
  LoadSfx(g_character);
  // reactor
  if (!rt.data.empty()) {
    auto& r = rt.data[0];
    fbs::ClientReactorT crt;
    crt.map_id = SceneSystemInstance::map_id_;
    crt.payload = std::make_unique<fbs::ReactorT>();
    crt.payload->reactor_index = r.r.index;
    crt.payload->delay = delay;
    ClientRequest::SendToHost(crt);
    return true;
  }
  if (shoot_weapon) {
    GameTriangle tri = {
        {
            SDL_FPoint{static_cast<float>(-shoot_range), -100},
            SDL_FPoint{static_cast<float>(-shoot_range), 100},
            SDL_FPoint{0, -28},
        },
    };
    auto cm = CharacterLogicSystem::RunAttackCheck(g_character, tri);
    auto cash_ball = PackageGameInstance::LoadActiveCashBall();
    std::u16string path;
    std::u16string effect;
    if (!cash_ball.empty()) {
      path = BallGameInstance::LoadCashBallPath(cash_ball);
      effect = BallGameInstance::LoadCashBallEffect(cash_ball);
    } else {
      path = BallGameInstance::LoadBallPath((*ball)->id);
      effect = u"Afterimage/hit.img/maceF";
    }
    ItemGameInstance::DecItemNum(*ball, 1);

    auto page = g_character.page;
    SDL_FPoint pos = g_character.pos;
    pos.y -= 28;
    SDL_FPoint goal = pos;
    if (g_character.flip) {
      goal.x += shoot_range;
    } else {
      goal.x -= shoot_range;
    }
    auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay, page,
                                                   700, path);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      cm.data[0].hits = {50};
      auto d = BallGameInstance::LoadBallTime(cct);
      auto cat = SkillGameInstance::CreateAttackPayload(cm, pos, d);
      cat.payload[0]->effect = {effect.begin(), effect.end()};
      ClientRequest::SendToHost(cat);
      // 只有弓/弩的飞行道具攻击才触发 Ultimate。
      TriggerBowFinalAttack(cm, d);
      TriggerCrossbowFinalAttack(cm, d);
    }
    ServerBallInstance::HandleServerB(cct.payload);
    ClientRequest::SendToHost(cct);
  } else {
    auto cm = RunAttackCheck(g_character, g_r, 1);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      cm.data[0].hits = {100};
      auto hit_type = AfterimageGameInstance::LoadHitType(g_character);
      auto effect =
          "Afterimage/hit.img/" + std::string{hit_type.begin(), hit_type.end()};
      auto cat = SkillGameInstance::CreateAttackPayload(cm, g_character.pos,
                                                        delay, 0, effect, "");
      ClientRequest::SendToHost(cat);
      RegisterFinalAttack(g_character, delay);
      RegisterPageFinalAttack(g_character, delay);
      RegisterSpearmanFinalAttack(g_character, delay);
    } else {
      SkillGameInstance::CreateAttackPayload(cm, g_character.pos, delay);
    }
  }
  for (auto& buff : SkillGameInstance::ski_) {
    if (buff.action) {
      buff.action(&buff, u"");
    }
  }
  return true;
}

bool CharacterLogicSystem::RunSeat(GameCharacter& g_character) {
  if (self_seat_.has_value()) {
    // 095 CField seat：坐着时再按坐下键/方向键/跳跃起身。
    bool stand = character_action_input_.contains("sit") ||
                 character_action_input_.contains("up") ||
                 character_action_input_.contains("left") ||
                 character_action_input_.contains("right") ||
                 character_action_input_.contains("jump");
    if (stand && self_sit_cooldown_ <= Window::dt_now_) {
      self_seat_.reset();
      self_fh_ = 0;
      self_hspeed_ = 0;
      self_vspeed_ = 0;
      self_sit_cooldown_ = Window::dt_now_ + 300;
      RunAction(g_character, u"jump");
      return true;
    }
    g_character.pos = *self_seat_;
    self_hspeed_ = 0;
    self_vspeed_ = 0;
    RunAction(g_character, u"sit");
    return true;
  }

  // 095 CField seat：站在座位附近按坐下键（默认 X）坐下。
  if (!character_action_input_.contains("sit") || self_fh_ == 0 ||
      self_sit_cooldown_ > Window::dt_now_) {
    return false;
  }
  for (const auto& seat : SeatGameInstance::data_) {
    if (std::abs(seat.pos.x - g_character.pos.x) <= 25.0f &&
        std::abs(seat.pos.y - g_character.pos.y) <= 30.0f) {
      self_seat_ = seat.pos;
      g_character.pos = seat.pos;
      self_fh_ = 0;
      self_hspeed_ = 0;
      self_vspeed_ = 0;
      self_sit_cooldown_ = Window::dt_now_ + 300;
      RunAction(g_character, u"sit");
      return true;
    }
  }
return false;
}

bool CharacterLogicSystem::RunFly(GameCharacter& g_character) {
  // 095：飞行地图 + 坐骑时进入飞行姿态（fly 动作），不受重力和落脚点约束。
  self_fly_ = true;
  self_fh_ = 0;
  self_lr_ = 0;
  RunAction(g_character, u"fly");

  auto dt = Window::delta_time_ / 1000.0f;
  auto max_h = CharacterStatGameInstance::GetHspeedMax();

  if (character_action_input_.contains("left")) {
    self_hspeed_ -= 1400.0f * dt;
  } else if (character_action_input_.contains("right")) {
    self_hspeed_ += 1400.0f * dt;
  } else {
    self_hspeed_ *= std::max(0.0f, 1.0f - 4.0f * dt);
  }
  self_hspeed_ = std::clamp(self_hspeed_, -max_h, max_h);

  if (character_action_input_.contains("up")) {
    self_vspeed_ -= 1200.0f * dt;
  } else if (character_action_input_.contains("down")) {
    self_vspeed_ += 1000.0f * dt;
  } else {
    self_vspeed_ *= std::max(0.0f, 1.0f - 4.0f * dt);
  }
  self_vspeed_ = std::clamp(self_vspeed_, -400.0f, 400.0f);

  g_character.pos.x += self_hspeed_ * dt;
  g_character.pos.y += self_vspeed_ * dt;

  auto border = MapInfoGameInstance::LoadMrBorder(
      SceneSystemInstance::map_id_);
  if (border.w > border.x && border.h > border.y) {
    g_character.pos.x = std::clamp(g_character.pos.x, border.x + 10, border.w - 10);
    g_character.pos.y = std::clamp(g_character.pos.y, border.y + 10, border.h - 10);
  }

  RunFlip(g_character);
  return true;
}

bool CharacterLogicSystem::RunSwim(GameCharacter& g_character) {
  // 095 CVecCtrl::IsSwimming 的水中运动：低重力、可上下控制、水平阻尼。
  self_swim_ = true;
  self_fh_ = 0;
  self_lr_ = 0;
  RunAction(g_character, u"fly");

  auto dt = Window::delta_time_ / 1000.0f;
  auto max_h = CharacterStatGameInstance::GetHspeedMax() * 0.6f;

  if (character_action_input_.contains("left")) {
    self_hspeed_ -= 900.0f * dt;
  } else if (character_action_input_.contains("right")) {
    self_hspeed_ += 900.0f * dt;
  } else {
    self_hspeed_ *= std::max(0.0f, 1.0f - 3.0f * dt);
  }
  self_hspeed_ = std::clamp(self_hspeed_, -max_h, max_h);

  if (character_action_input_.contains("up")) {
    self_vspeed_ -= 900.0f * dt;
  } else if (character_action_input_.contains("down")) {
    self_vspeed_ += 700.0f * dt;
  } else {
    // 轻微下沉，保持浮在水面附近。
    self_vspeed_ += 120.0f * dt;
  }
  self_vspeed_ = std::clamp(self_vspeed_, -220.0f, 200.0f);

  g_character.pos.x += self_hspeed_ * dt;
  g_character.pos.y += self_vspeed_ * dt;

  if (auto rect = MapInfoGameInstance::LoadSwimRect(
          SceneSystemInstance::map_id_);
      rect.has_value()) {
    g_character.pos.x =
        std::clamp(g_character.pos.x, rect->x + 10, rect->x + rect->w - 10);
    g_character.pos.y =
        std::clamp(g_character.pos.y, rect->y + 4, rect->y + rect->h - 4);
  }

  RunFlip(g_character);
  return true;
}

bool CharacterLogicSystem::RunPortal(GameCharacter& g_character) {
  // 特殊情况，如果掉到了地图外,直接传送到sp出生点
  auto border = MapInfoGameInstance::LoadMrBorder(SceneSystemInstance::map_id_);
  // MRBottom 是地图底边，角色正常站在底边时不能判定为掉出地图。
  constexpr float kFallOutMargin = 200.0f;
  if (border.h > border.y &&
      g_character.pos.y > border.h + kFallOutMargin) {
    // no change map
    auto sp = PortalGameInstance::data_.find(u"sp");
    if (sp == PortalGameInstance::data_.end()) {
      return false;
    }
    const auto& tn = sp->second;
    g_character.pos = tn.pos;
    g_character.pos.y -= 5;
    self_hspeed_ = 0;
    self_vspeed_ = 0;
    self_fh_ = 0;
    RunAction(g_character, u"jump");
    self_portal_cooldown_ = Window::dt_now_ + 1200;
    return true;
  }
  if (g_character.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    return false;
  }
  if (self_portal_cooldown_ <= Window::dt_now_) {
    bool up = character_action_input_.contains("up");
    for (const auto& por : PortalGameInstance::data_ | std::views::values) {
      if (por.tm == 999999999) {
        continue;
      }
      const auto& g_pos = g_character.pos;
      const auto& p_pos = por.pos;
      bool is_near =
          g_pos.x == std::clamp(g_pos.x, p_pos.x - 40, p_pos.x + 40) &&
          g_pos.y == std::clamp(g_pos.y, p_pos.y - 50, p_pos.y + 50);

      // 095 脚本传送门（pt 6=ps / 9=pcs）：执行已注册的同名脚本。
      if (is_near && (por.pt == 6 || por.pt == 9)) {
        auto& fns = Script::fns();
        if (auto it = fns.find(por.pn); it != fns.end()) {
          it->second(std::any{});
          self_portal_cooldown_ = Window::dt_now_ + 1000;
          return true;
        }
      }

      if (((por.pt == 1 || por.pt == 2) && up) || por.pt == 3) {
        if (is_near) {
          if (por.tm != SceneSystemInstance::map_id_) {
            // need to change map
            SceneSystemInstance::EnterPrepare(por.tm, por.tn, 0);
            self_portal_cooldown_ = Window::dt_now_ + 1000;
          } else {
            // no change map
            const auto& tn = PortalGameInstance::data_.find(por.tn)->second;
            g_character.pos = tn.pos;
            g_character.pos.y -= 5;
            self_hspeed_ = 0;
            self_vspeed_ = 0;
            self_fh_ = 0;
            RunAction(g_character, u"jump");
            self_portal_cooldown_ = Window::dt_now_ + 1200;
            AudioGameInstance::PlayNow(u"Game.img/Portal");
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool CharacterLogicSystem::RunFace(GameCharacter& g_character) {
  if (!character_face_input_.empty()) {
    auto& face = *character_face_input_.begin();
    RunFaceAction(g_character, std::u16string{face.begin(), face.end()});
    return true;
  }
  return false;
}

void CharacterLogicSystem::RunFaceAnimate(GameCharacter& g_character) {
  g_character.face.time += Window::delta_time_;
  const auto& delays = CharacterGameInstance::face_data_.at(g_character.face.id)
                           .delay.at(g_character.face.action);
  auto delay = delays[g_character.face.index];
  if (delay != 0) {
    if (g_character.face.time >= delay) {
      g_character.face.index += 1;
      if (g_character.face.index >= delays.size()) {
        if (g_character.face.action == u"blink" ||
            (g_character.face.destroy <= Window::dt_now_)) {
          g_character.face.action = u"default";
          g_character.face.destroy = Window::dt_now_ + 4000;
        }
        g_character.face.index = 0;
      }
      g_character.face.time = 0;
    }
    return;
  }
  if (g_character.face.destroy <= Window::dt_now_) {
    if (g_character.face.action == u"default") {
      g_character.face.action = u"blink";
    } else {
      g_character.face.action = u"default";
    }
    g_character.face.index = 0;
    g_character.face.time = 0;
    g_character.face.destroy = Window::dt_now_ + 4000;
  }
  return;
}

CharacterLogicSystem::PosType CharacterLogicSystem::LoadPosType(
    GameCharacter& g_character) {
  auto swim_r = MapInfoGameInstance::LoadSwimRect(SceneSystemInstance::map_id_);
  if (swim_r.has_value()) {
    if (SDL_PointInRectFloat(&g_character.pos, &swim_r.value())) {
      return PosType::kSwim;
    }
  } else {
    return PosType::kLand;
  }
  return PosType::kLand;
}

void CharacterLogicSystem::RunNetworkSyncState() {
  if (ccs_.payload.empty()) {
    return;
  }
  ccs_.map_id = SceneSystemInstance::map_id_;
  ClientRequest::SendToHost(ccs_);
  ccs_.payload.clear();
}

void CharacterLogicSystem::RunNetworkSyncAction() {
  const auto& sf = CharacterGameInstance::self_;
  auto map_id = SceneSystemInstance::map_id_;

  static std::u16string action;
  static bool action_animate;
  if (action != sf.action || action_animate != sf.action_animate) {
    fbs::ActionT a;
    a.action = {sf.action.begin(), sf.action.end()};
    a.action_animate = sf.action_animate;
    a.action_index = sf.action_index;

    fbs::ClientCharacterActionT cct;
    cct.map_id = map_id;
    cct.payload = std::make_unique<fbs::ActionT>(a);

    ClientRequest::SendToHost(cct);
    action = sf.action;
    action_animate = sf.action_animate;
  }
}
const static int32_t MIN_SEND_INTERVAL_MS = 33;

void CharacterLogicSystem::RunNetworkSyncPos() {
  const auto& sf = CharacterGameInstance::self_;
  auto map_id = SceneSystemInstance::map_id_;
  static SDL_FPoint pos;
  if ((sf.pos.x != pos.x || sf.pos.y != pos.y) && sf.action != u"dead") {
    fbs::MovementT mv;
    mv.x1 = pos.x;
    mv.y1 = pos.y;
    mv.x2 = sf.pos.x;
    mv.y2 = sf.pos.y;
    mv.page = sf.page;
    mv.time = std::min(Window::delta_time_, MIN_SEND_INTERVAL_MS);
    pos = sf.pos;
    fbs::ClientCharacterMvT mvt;
    mvt.map_id = map_id;
    mvt.payload = std::make_unique<fbs::MovementT>(mv);
    ClientRequest::SendToHost(mvt);
  }
}

void CharacterLogicSystem::RunNetworkSyncFlip() {
  const auto& sf = CharacterGameInstance::self_;
  auto map_id = SceneSystemInstance::map_id_;
  static bool flip;
  if (flip != sf.flip) {
    fbs::FlipT f;
    f.flip = sf.flip;
    fbs::ClientCharacterFlipT fpt;
    fpt.map_id = map_id;
    fpt.payload = std::make_unique<fbs::FlipT>(f);
    ClientRequest::SendToHost(fpt);
    flip = sf.flip;
  }
}

void CharacterLogicSystem::RunNetworkSyncFace() {
  const auto& sf = CharacterGameInstance::self_;
  auto map_id = SceneSystemInstance::map_id_;
  static std::u16string face;
  if (face != sf.face.action) {
    if (sf.face.action != u"blink") {
      fbs::FaceT ft;
      ft.face_action = {
          sf.face.action.begin(),
          sf.face.action.end(),
      };
      fbs::ClientCharacterFcT fct;
      fct.map_id = map_id;
      fct.payload = std::make_unique<fbs::FaceT>(ft);
      ClientRequest::SendToHost(fct);
      face = sf.face.action;
    }
  }
}

void CharacterLogicSystem::RunNetworkSyncSelf() {
  const auto& sf = CharacterGameInstance::self_;
  // 攻速（Booster 等）属于角色数据的一部分：变化时自动触发 CharacterT 同步，
  // 否则远程玩家按旧攻速播放你的攻击动画。
  static uint64_t last_id = 0;
  static int8_t last_attack_speed = 0;
  if (last_id != sf.id) {
    last_id = sf.id;
    last_attack_speed = sf.attack_speed;
  } else if (last_attack_speed != sf.attack_speed) {
    last_attack_speed = sf.attack_speed;
    cct_.map_id = SceneSystemInstance::map_id_;
  }

  if (cct_.map_id != 0) {
    auto c = ServerCharacterInstance::LoadCharactert(sf);
    cct_.payload = std::make_unique<fbs::CharacterT>(std::move(c));
    ClientRequest::SendToHost(cct_);
    cct_.map_id = 0;
  }
}

void CharacterLogicSystem::RunNetworkSyncHp() {
  static int32_t hp;
  auto sf_hp = CharacterStatGameInstance::GetHp();
  if (hp != sf_hp) {
    fbs::StateT st;
    st.state = fbs::StateEnum_HP;
    st.val = sf_hp;
    ccs_.payload.push_back(std::make_unique<fbs::StateT>(st));
    hp = sf_hp;
  }
}

void CharacterLogicSystem::RunNetworkSyncMaxHp() {
  static int32_t max_hp;
  auto sf_max_hp = CharacterStatGameInstance::GetHpMax();
  if (max_hp != sf_max_hp) {
    fbs::StateT st;
    st.state = fbs::StateEnum_MAX_HP;
    st.val = sf_max_hp;
    ccs_.payload.push_back(std::make_unique<fbs::StateT>(st));
    max_hp = sf_max_hp;
  }
}

void CharacterLogicSystem::RunNetworkSync() {
  RunNetworkSyncAction();
  RunNetworkSyncSelf();
  RunNetworkSyncHp();
  RunNetworkSyncMaxHp();
  RunNetworkSyncState();
  static uint64_t time = 0;
  // 节流：频率限制
  if (Window::dt_now_ - time <= MIN_SEND_INTERVAL_MS) {
    return;
  }
  time = Window::dt_now_;
  RunNetworkSyncPos();
  RunNetworkSyncFlip();
  RunNetworkSyncFace();
}

void CharacterLogicSystem::RunNetworkDieSync(GameCharacter& g_character) {
  fbs::DieT d;
  d.x = g_character.pos.x;
  d.y = g_character.pos.y;
  fbs::ClientCharacterDieT req;
  req.map_id = SceneSystemInstance::map_id_;
  req.payload = std::make_unique<fbs::DieT>(d);
  ClientRequest::SendToHost(req);
  return;
}

CharacterLogicSystem::ActionEnum CharacterLogicSystem::LoadActionType(
    GameCharacter& g_character) {
  const static std::flat_map<std::u16string, ActionEnum> MapName = {
      {u"stand1", ActionEnum::kStand},    {u"stand2", ActionEnum::kStand},
      {u"alert", ActionEnum::kAlert},     {u"walk1", ActionEnum::kWalk},
      {u"walk2", ActionEnum::kWalk},      {u"prone", ActionEnum::kProne},
      {u"jump", ActionEnum::kJump},       {u"ladder", ActionEnum::kClimb},
      {u"rope", ActionEnum::kClimb},      {u"proneStab", ActionEnum::kAttack},
      {u"stabO1", ActionEnum::kAttack},   {u"stabO2", ActionEnum::kAttack},
      {u"swingO1", ActionEnum::kAttack},  {u"swingO2", ActionEnum::kAttack},
      {u"swingO3", ActionEnum::kAttack},  {u"stabT1", ActionEnum::kAttack},
      {u"swingP1", ActionEnum::kAttack},  {u"shoot1", ActionEnum::kAttack},
      {u"shoot2", ActionEnum::kAttack},   {u"shootF", ActionEnum::kAttack},
      {u"swingT1", ActionEnum::kAttack},  {u"swingT2", ActionEnum::kAttack},
      {u"swingT3", ActionEnum::kAttack},  {u"shot", ActionEnum::kAttack},
      {u"stabT2", ActionEnum::kAttack},   {u"dead", ActionEnum::kDead},
      {u"alert2", ActionEnum::kSkill},    {u"alert3", ActionEnum::kSkill},
      {u"alert4", ActionEnum::kSkill},    {u"alert5", ActionEnum::kSkill},
      {u"assaulter", ActionEnum::kSkill}, {u"avenger", ActionEnum::kSkill},
      {u"burster1", ActionEnum::kSkill},  {u"burster2", ActionEnum::kSkill},
      {u"craft", ActionEnum::kSkill},     {u"crawl", ActionEnum::kSkill},
      {u"ladder2", ActionEnum::kSkill},   {u"magic1", ActionEnum::kSkill},
      {u"magic2", ActionEnum::kSkill},    {u"magic3", ActionEnum::kSkill},
      {u"prone2", ActionEnum::kSkill},    {u"rope2", ActionEnum::kSkill},
      {u"savage", ActionEnum::kSkill},    {u"shoot6", ActionEnum::kSkill},
      {u"shootDb1", ActionEnum::kSkill},  {u"shotC1", ActionEnum::kSkill},
      {u"sit", ActionEnum::kSit},         {u"fly", ActionEnum::kSkill},
      {u"rushO", ActionEnum::kSkill},
  };
  auto action = MapName.at(g_character.action);
  if (action == ActionEnum::kDead) {
    return action;
  }
  if (g_character.skill.has_value()) {
    if (action != ActionEnum::kSkill) {
      return action;
    }
    return ActionEnum::kSkill;
  }
  return action;
};

void CharacterLogicSystem::RunTomb(GameCharacter& g_character) {
  if (!g_character.tomb.has_value()) {
    return;
  }
  auto& tomb = g_character.tomb.value();
  tomb.pos.y += Window::delta_time_ * 0.4;
  tomb.pos.y = std::min(tomb.b.y, tomb.pos.y);
  tomb.ani_time += Window::delta_time_;
  auto node = WzResource::effect_->find(u"Tomb.img/" + tomb.ani_type);
  auto texture_node = node->get_child(std::to_string(tomb.ani_index));
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL>*>(texture_node)->get_uol();
  }
  auto delay =
      static_cast<wz::Property<int>*>(texture_node->get_child(u"delay"))->get();
  if (tomb.ani_time >= delay) {
    tomb.ani_time = 0;
    tomb.ani_index += 1;
    if (tomb.ani_index >= node->children_count()) {
      if (tomb.ani_type == u"fall") {
        tomb.ani_type = u"land";
      }
      tomb.ani_index = 0;
    }
  }

  const float velocity = 0.05f;  // 角速度
  const float radius = 10.0f;    // 半径
  g_character.pos.x = tomb.b.x + radius * std::cos(tomb.rotation);
  g_character.pos.y = tomb.b.y - 10 + radius * std::sin(tomb.rotation);
  tomb.rotation += velocity;
  if (tomb.rotation >= 2 * std::numbers::pi) {
    tomb.rotation -= 2 * std::numbers::pi;  // 保持角度在 [0, 2π) 范围内
  }
  return;
}

void CharacterLogicSystem::RunAnimateChair(GameCharacter& g_character) {
  if (!g_character.chair.has_value()) {
    return;
  }
  auto chair_id = g_character.chair->id;
}

void CharacterLogicSystem::RunStateMachine() {
  auto& g_character = CharacterGameInstance::self_;
  if (CraftUiSystem::Crafting()) {
    // 制作期间屏蔽移动、跳跃、攻击、技能和使用物品等动作。
    character_action_input_.clear();
    character_skill_input_.clear();
    character_item_input_.clear();
    self_hspeed_ = 0;
    self_vspeed_ = 0;

    const auto craft_action =
        (CharacterGameInstance::bone_data_.contains(u"craft") ||
         CharacterGameInstance::extern_action_.contains(u"craft"))
            ? u"craft"
            : u"alert";
    if (g_character.action != craft_action) {
      g_character.action = craft_action;
      g_character.action_index = 0;
      g_character.action_time = 0;
      g_character.action_animate = true;
      g_character.skill = std::nullopt;
    }
    RunFace(g_character);
    RunFaceAnimate(g_character);
    RunAnimate(g_character);
    RunNetworkSync();
    return;
  }
  auto o_character = g_character;

  // 095 CVecCtrl::IsSwimming：在游泳地图的水域内切换到游泳状态，
  // 不参与落脚点碰撞，离开水面后恢复普通下落。
  // 角色骨骼没有 swim 动作，095/本数据包都用 fly 动作表现水中/飞行姿态。
  if (MapInfoGameInstance::IsSwimming(SceneSystemInstance::map_id_,
                                      g_character.pos)) {
    RunFace(g_character);
    RunFaceAnimate(g_character);
    RunSwim(g_character);
    RunAnimate(g_character);
    RunNetworkSync();
    return;
  }

  // 095 飞行状态：仅在飞行地图且骑乘时启用（数据驱动的飞行技能可复用）。
  if (MapInfoGameInstance::IsFlyingMap(SceneSystemInstance::map_id_) &&
      !g_character.mount.empty()) {
    RunFace(g_character);
    RunFaceAnimate(g_character);
    RunFly(g_character);
    RunAnimate(g_character);
    RunNetworkSync();
    return;
  }
  if (self_fly_) {
    self_fly_ = false;
    self_fh_ = 0;
    self_vspeed_ = 0;
    RunAction(g_character, u"jump");
  }
  if (self_swim_) {
    self_swim_ = false;
    self_fh_ = 0;
    self_vspeed_ = 0;
    RunAction(g_character, u"jump");
  }

  RunItem(g_character);
  auto g_action = LoadActionType(g_character);
  RunFace(g_character);
  RunFaceAnimate(g_character);
  switch (g_action) {
    case ActionEnum::kStand:
    case ActionEnum::kAlert:
    case ActionEnum::kWalk: {
      RunAnimate(g_character);
      RunFlip(g_character);
      RunPick(g_character);
      if (RunHarvest(g_character)) {
        break;
      }
      if (RunClimb(g_character)) {
        break;
      }
      if (RunSeat(g_character)) {
        break;
      }
      if (RunProne(g_character)) {
        break;
      }
      if (!RunWalk(g_character)) {
        RunAction(g_character, u"jump");
        break;
      }
      if (RunAttack(g_character)) {
        break;
      }
      if (RunSkill(g_character)) {
        break;
      }
      if (RunPortal(g_character)) {
        break;
      }
      RunJump(g_character);
      break;
    }
    case ActionEnum::kProne: {
      RunFlip(g_character);
      RunPick(g_character);
      if (!RunProne(g_character)) {
        RunStandAction(g_character);
      }
      if (RunJump(g_character)) {
        break;
      }
      if (RunClimb(g_character)) {
        break;
      }
      if (RunAttack(g_character)) {
        break;
      }
      if (RunSkill(g_character)) {
        break;
      }
      if (RunPortal(g_character)) {
        break;
      }
      break;
    }
    case ActionEnum::kJump: {
      RunFlip(g_character);
      if (!RunFall(g_character)) {
        // 刚落地后，瞬间动作不一定是stand，需要再进行一次状态机
        RunStandAction(g_character);
        RunStateMachine();
        break;
      }
      if (RunAttack(g_character)) {
        break;
      }
      if (RunSkill(g_character)) {
        break;
      }
      if (RunClimb(g_character)) {
        break;
      }
      if (RunPortal(g_character)) {
        break;
      }
      if (RunJump(g_character)) {
        break;
      }
      break;
    }
    case ActionEnum::kClimb: {
      RunAnimate(g_character);
      RunFlip(g_character);
      if (RunJump(g_character)) {
        break;
      }
      if (!RunClimbing(g_character)) {
        RunAction(g_character, u"jump");
      }
      if (RunPortal(g_character)) {
        break;
      }
      if (RunSkill(g_character)) {
        break;
      }
      break;
    }
    case ActionEnum::kSit: {
      // 地图座位（SeatGameInstance）：坐着时由 RunSeat 处理起身。
      if (self_seat_.has_value()) {
        RunSeat(g_character);
        break;
      }
      if (!RunSitting(g_character)) {
        RunStateMachine();
      }
      break;
    }
    case ActionEnum::kSkill:
    case ActionEnum::kAttack: {
      bool fall = RunFall(g_character);
      if (RunAnimate(g_character)) {
        // 冷却
        self_flip_cooldown_ = Window::dt_now_ + 33;
        self_attack_cooldown_ = Window::dt_now_ + 33;
        if (!fall) {
          // 落地
          self_vspeed_ = 0;
          self_hspeed_ = 0;
          RunStandAction(g_character);
        } else {
          RunAction(g_character, u"jump");
        }
        g_character.skill = std::nullopt;
        RunNetworkSync();
        RunStateMachine();
      }
      break;
    }
    case ActionEnum::kDead: {
      RunTomb(g_character);
      break;
    }
    default: {
      std::abort();
    }
  }
  RunNetworkSync();
}

void CharacterLogicSystem::RunOthersMv() {
  for (auto& c : CharacterGameInstance::others_ | std::views::values) {
    auto& mvs = c.mvs;
    if (mvs.empty()) {
      continue;
    }
    auto mv = mvs[0];
    auto per = Window::delta_time_ / (float)mv.time;
    // 计算当前点在线段上的进度（基于 x 和 y）
    float dx = mv.x2 - mv.x1;
    float dy = mv.y2 - mv.y1;
    float length_sq = dx * dx + dy * dy;

    if (length_sq > 0) {
      // 投影参数 t：当前点在线段上的位置（0=起点，1=终点）
      float t = ((c.g_character.pos.x - mv.x1) * dx +
                 (c.g_character.pos.y - mv.y1) * dy) /
                length_sq;
      t = std::clamp(t, 0.0f, 1.0f);
      per += t;
    }
    per = std::min(per, 1.0f);

    auto per_x = mv.x1 + (mv.x2 - mv.x1) * per;
    auto per_y = mv.y1 + (mv.y2 - mv.y1) * per;
    c.g_character.pos.x = per_x;
    c.g_character.pos.y = per_y;
    c.g_character.page = mv.page;
    if (per >= 1.0f) {
      mvs.erase(mvs.begin());
    }
  }
}

void CharacterLogicSystem::RunOthersStateMachine() {
  for (auto& c : CharacterGameInstance::others_ | std::views::values) {
    auto& g_character = c.g_character;
    RunFaceAnimate(g_character);
    auto g_action = LoadActionType(g_character);
    switch (g_action) {
      case ActionEnum::kStand:
      case ActionEnum::kAlert:
      case ActionEnum::kWalk:
      case ActionEnum::kClimb: {
        RunAnimate(g_character);
        break;
      }
      case ActionEnum::kAttack:
      case ActionEnum::kSkill: {
        auto action_index = g_character.action_index;
        auto action_time = g_character.action_time;
        if (RunAnimate(g_character)) {
          g_character.skill = std::nullopt;
          // 一次性动作,但是不知道后续是什么动作,保留动作
          g_character.action_index = action_index;
          g_character.action_time = action_time;
          g_character.action_animate = false;
        } else {
          g_character.action_animate = true;
        }
        break;
      }
      case ActionEnum::kDead: {
        RunTomb(g_character);
        break;
      }
      case ActionEnum::kSit: {
        RunAnimateChair(g_character);
        break;
      }
      default: {
        break;
      }
    }
  }
}

void CharacterLogicSystem::RunOthers() {
  RunOthersStateMachine();
  RunOthersMv();
}

void CharacterLogicSystem::RunColor() {
  auto g_character = &CharacterGameInstance::self_;
  auto cooldown = CharacterLogicSystem::self_invincible_cooldown_;
  if (cooldown >= Window::dt_now_) {
    auto delta = cooldown - Window::dt_now_;
    bool dark = (delta % 200) > 100;
    Uint8 color = dark ? 128 : 255;
    g_character->color = {color, color, color, 255};
  } else {
    g_character->color = {255, 255, 255, 255};
  }
}

// 人物状态机
bool CharacterLogicSystem::Run() {
  RunOthers();
  RunStateMachine();
  RunRecovery(CharacterGameInstance::self_);

  // 地图附加规则（095 服务端字段，本地数据驱动）：
  // decHP 每 10 秒扣血；timeLimit 到期后强制返回 forcedReturn/returnMap。
  const auto map_id = SceneSystemInstance::map_id_;
  const auto now = Window::dt_now_;
  static uint32_t dec_hp_map = 0;
  static uint64_t dec_hp_time = 0;
  if (map_id != dec_hp_map) {
    dec_hp_map = map_id;
    dec_hp_time = now + 10000;
  }
  if (auto dec_hp = MapInfoGameInstance::LoadDecHP(map_id);
      dec_hp > 0 && now >= dec_hp_time) {
    dec_hp_time = now + 10000;
    auto hp = CharacterStatGameInstance::GetHp();
    auto new_hp = std::max(0, hp - dec_hp);
    if (new_hp < hp) {
      CharacterStatGameInstance::SetHp(new_hp);
      auto& self = CharacterGameInstance::self_;
      fbs::AttackT at;
      at.x = self.pos.x;
      at.y = self.pos.y - 30;
      // 人物受伤统一用紫色数字。
      at.type = static_cast<uint8_t>(DamageData::DamageColor::kPurple);
      at.num = static_cast<uint64_t>(hp - new_hp);
      ServerMobInstance::HandleSAttack(0, at);
      // 广播给同地图其他玩家。
      fbs::ClientMobAttackT cma;
      cma.map_id = SceneSystemInstance::map_id_;
      cma.payload = std::make_unique<fbs::AttackT>(at);
      ClientRequest::SendToHost(cma);
      if (new_hp == 0) {
        RunDieAction(self);
      }
    }
  }

  static uint32_t tl_map = 0;
  static uint64_t tl_end = 0;
  if (map_id != tl_map) {
    tl_map = map_id;
    auto limit = MapInfoGameInstance::LoadTimeLimit(map_id);
    tl_end = limit != 0 ? now + static_cast<uint64_t>(limit) * 1000 : 0;
  }
  if (tl_end != 0 && now >= tl_end) {
    tl_end = 0;
    auto target = MapInfoGameInstance::LoadForcedReturn(map_id);
    if (target == 0) {
      target = MapInfoGameInstance::LoadReturnMap(map_id);
    }
    if (target != 0 && target != map_id) {
      SceneSystemInstance::EnterPrepare(target, u"sp", 0);
    }
  }

  RunColor();
  // 怪物/技能挂的 debuff：持续伤害（中毒/灼烧）。
  RunDebuffDamage();
  // 角色状态（怪物技能等挂的 debuff）到期清理
  std::erase_if(CharacterGameInstance::self_.debuffs, [](const auto& kv) {
    return kv.second.destroy != 0 && kv.second.destroy <= Window::dt_time_;
  });
  for (auto& c : CharacterGameInstance::others_ | std::views::values) {
    std::erase_if(c.g_character.debuffs, [](const auto& kv) {
      return kv.second.destroy != 0 && kv.second.destroy <= Window::dt_time_;
    });
  }
  return true;
}

void CharacterLogicSystem::RunDebuffDamage() {
  auto& sf = CharacterGameInstance::self_;
  if (sf.action == u"dead") {
    self_poison_next_ = 0;
    self_burn_next_ = 0;
    return;
  }
  const auto now = Window::dt_now_;
  const auto now_ms = Window::dt_time_;

  // 095 CMob::Update：中毒每 1000ms 扣一次 nPoison；灼烧按 2000ms 结算。
  // 持续伤害不吃受击无敌期，也不触发受击动作/击退。
  const auto tick = [&](GameDebuff::DebuffType type, uint64_t interval,
                        uint64_t& next) {
    auto it = sf.debuffs.find(type);
    if (it == sf.debuffs.end()) {
      next = 0;
      return;
    }
    const auto& db = it->second;
    if (db.destroy != 0 && db.destroy <= now_ms) {
      next = 0;
      return;
    }
    if (next == 0) {
      next = now + interval;
      return;
    }
    if (now < next) {
      return;
    }
    next = now + interval;

    const auto damage = std::max<int32_t>(1, static_cast<int32_t>(db.val));
    const auto hp = CharacterStatGameInstance::GetHp();
    const auto new_hp = std::max<int32_t>(0, hp - damage);
    if (new_hp >= hp) {
      return;
    }
    CharacterStatGameInstance::SetHp(new_hp);

    fbs::AttackT at;
    at.x = sf.pos.x;
    at.y = sf.pos.y - 30;
    // 人物受伤统一用紫色数字。
    at.type = static_cast<uint8_t>(DamageData::DamageColor::kPurple);
    at.num = static_cast<uint64_t>(hp - new_hp);
    ServerMobInstance::HandleSAttack(0, at);

    // 广播给同地图其他玩家（服务端 HandleAttack 转发）。
    fbs::ClientMobAttackT cma;
    cma.map_id = SceneSystemInstance::map_id_;
    cma.payload = std::make_unique<fbs::AttackT>(at);
    ClientRequest::SendToHost(cma);

    if (new_hp == 0) {
      RunDieAction(sf);
    }
  };

  tick(GameDebuff::DebuffType::kPoison, 1000, self_poison_next_);
  tick(GameDebuff::DebuffType::kBurn, 2000, self_burn_next_);
}

void CharacterLogicSystem::RunDieAction(GameCharacter& g_character) {
  RunAction(g_character, u"dead");
  if (self_fh_ == 0 || self_lr_ == 0) {
    auto o_character = g_character;
    // 空中
    self_hspeed_ = 0;
    self_vspeed_ = CharacterStatGameInstance::GetVspeedMax();
    //
    auto map_id = SceneSystemInstance::map_id_;
    auto border = MapInfoGameInstance::LoadMrBorder(map_id);

    SDL_FPoint new_pos{
        .x = g_character.pos.x,
        .y = border.h,
    };
    auto inter_pos = Physic::FallIntersectPos(g_character.pos, new_pos,
                                              FootholdGameInstance::data_);
    g_character.pos = inter_pos.begin()->second.pos;
  }
  GameTomb t{
      .ani_type = u"fall",
      .pos = {g_character.pos.x, g_character.pos.y - 300},
      .b = g_character.pos,
  };
  g_character.tomb = t;

  self_invincible_cooldown_ = 0;
  self_vspeed_ = 0;
  self_hspeed_ = 0;
  self_alert_cooldown_ = 0;

  RunNetworkDieSync(g_character);

  AudioGameInstance::PlayNow(u"Game.img/Tombstone");

  ReviveUiSystem::Toggle();
}

void CharacterLogicSystem::LoadSfx(GameCharacter& g_character) {
  if (!g_character.weapon.has_value()) {
    return;
  }
  auto delay = AfterimageGameInstance::LoadBeatTime(g_character);
  auto g_weapon = g_character.weapon->id;
  auto g_weapon_info = EquipGameInstance::LoadEquipInfo(g_weapon);
  auto sfx = static_cast<wz::Property<std::u16string>*>(
                 g_weapon_info->get_child(u"sfx"))
                 ->get();
  auto WeaponType = EquipGameInstance::LoadWeaponType(g_character);
  auto action_type = LoadActionType(g_character);
  if (!(action_type == CharacterLogicSystem::ActionEnum::kSkill ||
        action_type == CharacterLogicSystem::ActionEnum::kAttack)) {
    return;
  }
  if (g_character.skill.has_value() &&
      WeaponType == EquipGameInstance::WeaponType::kWand) {
    return;
  }
  if (weapon_attack_action2.contains(WeaponType)) {
    auto actions = &weapon_attack_action2.at(WeaponType);
    if (actions->contains(g_character.action) ||
        g_character.action == u"proneStab") {
      sfx = sfx + u"/Attack";
    } else {
      actions = &weapon_attack_action.at(WeaponType);
      if (actions->contains(g_character.action)) {
        sfx = sfx + u"/Attack2";
      } else {
        return;
      }
    }
  } else {
    auto actions = &weapon_attack_action.at(WeaponType);
    if (actions->contains(g_character.action) ||
        g_character.action == u"proneStab") {
      sfx = sfx + u"/Attack";
    } else {
      return;
    }
  }
  AudioGameInstance::Play(u"Weapon.img/" + sfx, delay);
  if (g_character.skill.has_value()) {
    // hard code
    if (g_character.skill == u"4001003") {
      AudioGameInstance::Play(u"Weapon.img/" + sfx, delay + 120);
    }
  }
}

float CharacterLogicSystem::LoadAttackSpeed(GameCharacter& g_character) {
  if (!g_character.weapon.has_value()) {
    return 1.0;
  }
  auto w_speed = EquipGameInstance::LoadEquipInc(g_character.weapon->id)
                     .at(EquipGameInstance::IncType::kWeaponSpeed);
  float speed = w_speed + g_character.attack_speed;
  return 1.6 - speed / 10;
}

void CharacterLogicSystem::RunSitChair(GameCharacter& g_character,
                                       const std::u16string& id) {
  if (IsAlert()) {
    return;
  }
  RunAction(g_character, u"sit");
  g_character.chair = GameChair{};
  g_character.chair->id = id;
  if (&g_character == &CharacterGameInstance::self_) {
    fbs::StateT st;
    st.state = fbs::StateEnum_BUFF_ITEM;
    st.val = std::stoi(std::string{id.begin(), id.end()});
    st.sub_val = 1;
    ccs_.payload.push_back(std::make_unique<fbs::StateT>(st));
  }

  return;
}

void CharacterLogicSystem::RunUnsitChair(GameCharacter& g_character) {
  if (g_character.chair.has_value()) {
    RunStandAction(g_character);
    if (&g_character == &CharacterGameInstance::self_) {
      fbs::StateT st;
      st.state = fbs::StateEnum_BUFF_ITEM;
      auto tmp = std::string{
          g_character.chair->id.begin(),
          g_character.chair->id.end(),
      };
      st.val = std::stoi(tmp);
      st.sub_val = 0;
      ccs_.payload.push_back(std::make_unique<fbs::StateT>(st));
    }
  }
  g_character.chair = std::nullopt;
  return;
}

void CharacterLogicSystem::RunMount(GameCharacter& g_character,
                                    const std::u16string& mount_id) {
  // 095：骑乘与椅子/变身互斥（alert 检查在使用入口做，这里服务端
    // 同步远程玩家时不能再依赖本地 self 的状态）。
  if (!g_character.morph.empty() || !g_character.mount.empty()) {
    return;
  }
  RunUnsitChair(g_character);

  // 形态 id：坐骑道具 → TamingMob 形态；本项目精简客户端的
  // TamingMob_000.wz 只有 0001~0025/1000（无 0190xxxx 完整贴图），
  // 找不到时回退 1000（speed=200 / jump=120 / fatigue=5）。
  std::u16string form = mount_id.size() >= 8 ? mount_id.substr(4) : u"1000";
  auto* info = WzResource::taming_mob_->find(form + u".img/info");
  if (info == nullptr || info->get_child(u"speed") == nullptr) {
    form = u"1000";
    info = WzResource::taming_mob_->find(form + u".img/info");
  }
  if (info == nullptr || info->get_child(u"speed") == nullptr) {
    return;  // 形态数据缺失，不上骑。
  }
  g_character.mount = form;
  g_character.mount_item = mount_id;
  g_character.action_index = 0;
  g_character.action_time = 0;

  if (&g_character == &CharacterGameInstance::self_) {
    // 095：骑乘特效（Skill/35001002 的 special/effect）；本项目无该素材，
    // 用音效代替。状态广播走 BUFF_ITEM（同椅子/变身模式）。
    AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");
    CharacterStatGameInstance::Update();

    fbs::StateT st;
    st.state = fbs::StateEnum_BUFF_ITEM;
    st.val = std::stoi(std::string{mount_id.begin(), mount_id.end()});
    st.sub_val = 1;
    ccs_.payload.push_back(std::make_unique<fbs::StateT>(st));

    st.state = fbs::StateEnum_ITEM_USE;
    st.sub_val = 1;
    ccs_.payload.push_back(std::make_unique<fbs::StateT>(st));
  }
  return;
}

void CharacterLogicSystem::RunUnmount(GameCharacter& g_character) {
  if (g_character.mount.empty()) {
    return;
  }
  // 先保存坐骑道具 id 再清（下骑回执要用）。
  std::u16string mount_id = g_character.mount_item;
  g_character.mount = u"";
  g_character.mount_item = u"";
  self_fly_ = false;
  g_character.action_index = 0;
  g_character.action_time = 0;

  if (&g_character == &CharacterGameInstance::self_) {
    AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");
    CharacterStatGameInstance::Update();

    // 下骑状态：回执坐骑道具 id + sub_val=0（同椅子解除模式），
    // 远程端才能正确走 TamingMob 分支清除。
    fbs::StateT st;
    st.state = fbs::StateEnum_BUFF_ITEM;
    st.val = std::stoi(std::string{mount_id.begin(), mount_id.end()});
    st.sub_val = 0;
    ccs_.payload.push_back(std::make_unique<fbs::StateT>(st));

    st.state = fbs::StateEnum_ITEM_USE;
    st.sub_val = 0;
    ccs_.payload.push_back(std::make_unique<fbs::StateT>(st));
  }
  return;
}

void CharacterLogicSystem::RunRecovery(GameCharacter& g_character) {
  if (&g_character != &CharacterGameInstance::self_ ||
      g_character.action == u"dead") {
    self_recovery_time_ = 0;
    self_mp_recovery_time_ = 0;
    return;
  }
  const uint64_t now = Window::dt_now_;

  // 2000000 Improved MP Recovery：每 10 秒自然恢复 x% Max MP，
  // 不受站立/移动限制。
  const int mp_recovery_lv = JobSkillGameInstance::LoadSkiLevel(u"2000000");
  if (mp_recovery_lv > 0) {
    const int percent = std::clamp(
        SkillGameInstance::LoadSkiLevelInt(
            u"2000000", static_cast<uint8_t>(mp_recovery_lv), u"x", 1),
        0, 100);
    if (self_mp_recovery_time_ == 0) {
      self_mp_recovery_time_ = now + 10000;
    } else if (now >= self_mp_recovery_time_) {
      self_mp_recovery_time_ = now + 10000;
      const auto mp = CharacterStatGameInstance::GetMp();
      const auto mp_max = CharacterStatGameInstance::GetMpMax();
      const int32_t gain = std::max<int32_t>(
          1,
          static_cast<int32_t>(static_cast<int64_t>(mp_max) * percent / 100));
      const auto new_mp = std::min(mp_max, mp + gain);
      if (new_mp > mp) {
        CharacterStatGameInstance::SetMp(new_mp);
        fbs::AttackT at;
        at.x = g_character.pos.x - 10;
        at.y = g_character.pos.y - 50;
        at.type = static_cast<uint8_t>(DamageData::DamageColor::kBlue);
        at.num = static_cast<uint64_t>(new_mp - mp);
        ServerMobInstance::HandleSAttack(0, at);
      }
    }
  } else {
    self_mp_recovery_time_ = 0;
  }

  auto action = LoadActionType(g_character);
  const bool standing =
      action == ActionEnum::kStand || action == ActionEnum::kAlert;
  const bool sitting = action == ActionEnum::kSit;
  // 095 is_stand_action 包含绳/梯上的静止动作，绳子上也允许自然恢复。
  const bool climbing = action == ActionEnum::kClimb;
  // 095 Improved HP Recovery：学会后不只在站立/坐下时自然恢复。
  const bool improved_hp_recovery =
      JobSkillGameInstance::LoadSkiLevel(u"1000000") > 0;
  if (!standing && !sitting && !climbing && !improved_hp_recovery) {
    self_recovery_time_ = 0;
    return;
  }
  // 095 TryRecovery：位置变化的帧不恢复，重新累计 10 秒。
  if (self_recovery_pos_.x != g_character.pos.x ||
      self_recovery_pos_.y != g_character.pos.y) {
    self_recovery_pos_ = g_character.pos;
    self_recovery_time_ = now + 10000;
    return;
  }
  if (self_recovery_time_ == 0) {
    self_recovery_time_ = now + 10000;
    return;
  }
  if (now < self_recovery_time_) {
    return;
  }
  self_recovery_time_ = now + 10000;

  // 095 CWvsContext::TryRecovery：
  // 地图 recovery 倍率 × 站立 10 HP / 3 MP；坐下 ×1.5。
  // 椅子自带 recoveryHP / recoveryMP 时直接使用椅子数值。
  // 095 还会把披风（aEquipped[5]）的 recovery 属性乘到地图恢复倍率上。
  double rate = std::max(
      0.0, MapInfoGameInstance::LoadRecovery(SceneSystemInstance::map_id_));
  if (rate > 1.0 && g_character.cape.has_value()) {
    auto* equip_info = EquipGameInstance::LoadEquipInfo(g_character.cape->id);
    if (equip_info != nullptr) {
      auto* node = equip_info->get_child(u"recovery");
      if (node != nullptr && node->type == wz::Type::Double) {
        rate *= static_cast<wz::Property<double>*>(node)->get();
      } else if (node != nullptr && node->type == wz::Type::Int) {
        rate *=
            static_cast<double>(static_cast<wz::Property<int>*>(node)->get());
      }
    }
  }
  int32_t hp_gain = 0;
  int32_t mp_gain = 0;
  if (sitting && g_character.chair.has_value()) {
    auto* info = ItemGameInstance::LoadItemInfo(g_character.chair->id, 0);
    if (info != nullptr) {
      auto load = [&](const char16_t* key) -> int32_t {
        auto* node = info->get_child(key);
        if (node == nullptr || node->type != wz::Type::Int) {
          return 0;
        }
        return static_cast<wz::Property<int>*>(node)->get();
      };
      hp_gain = load(u"recoveryHP");
      mp_gain = load(u"recoveryMP");
    }
  }
  if (hp_gain <= 0) {
    hp_gain = static_cast<int32_t>(std::trunc(rate * (sitting ? 15.0 : 10.0)));
  }
  if (mp_gain <= 0) {
    mp_gain = static_cast<int32_t>(std::trunc(rate * (sitting ? 4.5 : 3.0)));
  }
  auto hp = CharacterStatGameInstance::GetHp();
  auto mp = CharacterStatGameInstance::GetMp();
  const auto new_hp =
      std::min(CharacterStatGameInstance::GetHpMax(), hp + hp_gain);
  const auto new_mp =
      std::min(CharacterStatGameInstance::GetMpMax(), mp + mp_gain);
  CharacterStatGameInstance::SetHp(new_hp);
  CharacterStatGameInstance::SetMp(new_mp);

  // 095 CUser::MakeIncDecHPEffect：头顶蓝色恢复数字。
  if (new_hp > hp) {
    fbs::AttackT at;
    at.x = g_character.pos.x - 10;
    at.y = g_character.pos.y - 30;
    at.type = static_cast<uint8_t>(DamageData::DamageColor::kBlue);
    at.num = static_cast<uint64_t>(new_hp - hp);
    ServerMobInstance::HandleSAttack(0, at);
  }
}


void CharacterLogicSystem::RunItem(GameCharacter& g_character) {
  if (character_item_input_.empty()) {
    return;
  }
  for (const auto& input : character_item_input_) {
    auto item_id = input.val;
    std::u16string item_id2{item_id.begin(), item_id.end()};
    auto itm = PackageGameInstance::LoadItem(item_id2);
    if (itm == nullptr) {
      continue;
    }
    // 095 SendStatChangeItemUseRequest：alert 期间不能用 236 类属性道具。
    if (IsAlert() && item_id2.starts_with(u"0236")) {
      continue;
    }
    // 冷却统一由 ItemGameInstance::UseItem 管理。
    ItemGameInstance::UseItem(*itm);
  }
}

std::u16string CharacterLogicSystem::LoadMorphType(GameCharacter& g_character) {
  auto action_type = LoadActionType(g_character);
  switch (action_type) {
    case ActionEnum::kStand: {
      return u"stand";
    }
    case ActionEnum::kAlert: {
      return u"stand";
    }
    case ActionEnum::kWalk: {
      return u"walk";
    }
    case ActionEnum::kProne: {
      return u"prone";
    }
    case ActionEnum::kJump: {
      return u"jump";
    }
    case ActionEnum::kClimb: {
      return g_character.action;
    }
    default: {
      return u"jump";
      break;
    }
  }
  return u"";
}

void CharacterLogicSystem::RunBeingHit(float x, uint64_t num) {
  auto& sf = CharacterGameInstance::self_;
  sf.action_animate = true;
  auto action_type = CharacterLogicSystem::LoadActionType(sf);
  switch (action_type) {
    case CharacterLogicSystem::ActionEnum::kStand:
    case CharacterLogicSystem::ActionEnum::kAlert:
    case CharacterLogicSystem::ActionEnum::kWalk:
    case CharacterLogicSystem::ActionEnum::kClimb:
    case CharacterLogicSystem::ActionEnum::kProne: {
      CharacterLogicSystem::RunAction(sf, u"jump");
      self_fall_fh_ = self_fh_;
      break;
    }
    case CharacterLogicSystem::ActionEnum::kSit: {
      CharacterLogicSystem::RunUnsitChair(sf);
      CharacterLogicSystem::RunAction(sf, u"jump");
      self_fall_fh_ = self_fh_;
      break;
    }
    default: {
      break;
    }
  }
  CharacterLogicSystem::self_fh_ = 0;
  CharacterLogicSystem::self_lr_ = 0;
  CharacterLogicSystem::self_invincible_cooldown_ = Window::dt_now_ + 2000;
  CharacterLogicSystem::self_alert_cooldown_ = Window::dt_now_ + 5000;

  const auto speed = 270.0f;
  if (sf.pos.x > x) {
    if (CharacterLogicSystem::self_hspeed_ > 0 &&
        CharacterLogicSystem::self_hspeed_ <= speed) {
      CharacterLogicSystem::self_hspeed_ = speed;
    } else {
      CharacterLogicSystem::self_hspeed_ += speed;
    }
  } else {
    if (CharacterLogicSystem::self_hspeed_ <= 0 &&
        CharacterLogicSystem::self_hspeed_ > -speed) {
      CharacterLogicSystem::self_hspeed_ = -speed;
    } else {
      CharacterLogicSystem::self_hspeed_ -= speed;
    }
  }
  if (CharacterLogicSystem::self_vspeed_ < 0) {
    if (CharacterLogicSystem::self_vspeed_ > -speed) {
      CharacterLogicSystem::self_vspeed_ = -speed;
    }
  } else {
    CharacterLogicSystem::self_vspeed_ -= speed;
  }
  auto sf_hp = CharacterStatGameInstance::GetHp();
  sf_hp -= num;
  if (sf_hp <= 0) {
    CharacterStatGameInstance::SetHp(0);
    CharacterLogicSystem::RunDieAction(sf);
  } else {
    CharacterStatGameInstance::SetHp(sf_hp);
    CharacterLogicSystem::RunFaceAction(sf, u"hit");
  }
  return;
}

std::vector<uint64_t> CharacterLogicSystem::RunBuffCheck(
    GameCharacter& g_character, SDL_FRect g_r) {
  // 095 组队增益技能：作用对象是"队伍成员"，不是同地图所有玩家。
  //   - 施放者自己始终受益（party buff 对施放者本人也生效）；
  //   - 队友在技能范围内才生效；技能没有范围数据（LoadSkiR 返回空矩形）
  //     时按全队生效，避免精简包缺 Skill.wz 时组队技能完全失效。
  std::vector<uint64_t> r;
  const uint64_t self_id = ServerMain::local_addr_;
  const bool has_range = g_r.w > 0.0f && g_r.h > 0.0f;

  r.push_back(self_id);

  if (!PartyGameInstance::data_.has_value()) {
    return r;
  }
  for (const auto& m : PartyGameInstance::data_->members) {
    const uint64_t cid = m.player.client_id;
    if (cid == 0 || cid == self_id) {
      continue;
    }
    // 不在当前地图的队友拿不到 buff（095 队伍跨地图不共享范围增益）。
    auto it = CharacterGameInstance::others_.find(cid);
    if (it == CharacterGameInstance::others_.end()) {
      continue;
    }
    if (!has_range) {
      r.push_back(cid);
      continue;
    }
    auto c_r = LoadRect(it->second.g_character);
    if (SDL_HasRectIntersectionFloat(&c_r, &g_r)) {
      r.push_back(cid);
    }
  }
  return r;
}

}  // namespace sdlms
