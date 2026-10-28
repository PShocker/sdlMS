#include "effect_logic_system.h"
#include "character_logic_system.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_region_skill.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <cstdint>
#include <ranges>
#include <string>

bool EffectLogicSystem::RunDamage(GameEffect &g_effect) {
  bool r = false;
  auto current_time = Window::dt_time_;
  if (g_effect.delay >= current_time) {
    return r;
  }
  if (g_effect.z == false) {
    auto &Mobs = MobGameInstance::data_;
    auto &mob = Mobs.at(g_effect.time).mob;
    g_effect.pos = SDL_FPoint{
        g_effect.pos->x + mob.pos.x,
        g_effect.pos->y + mob.pos.y,
    };
    g_effect.z = true;
  }
  int32_t a = g_effect.alpha - Window::delta_time_ / 3.5f;
  if (a <= 0) {
    r = true;
  } else {
    g_effect.alpha = a;
  }
  return r;
}

bool EffectLogicSystem::RunEffect(GameEffect &g_effect) {
  bool r = false;
  auto current_time = Window::dt_time_;
  if (g_effect.delay >= current_time) {
    return r;
  }
  g_effect.time += Window::delta_time_;
  wz::Node *node;
  switch ((GameEffect::EffectSubType)g_effect.lv) {
  case GameEffect::EffectSubType::kCharacter: {
    node = WzResource::character_->find(g_effect.id);
    break;
  }
  case GameEffect::EffectSubType::kEffect: {
    node = WzResource::effect_->find(g_effect.id);
    break;
  }
  case GameEffect::EffectSubType::kItem: {
    node = WzResource::item_->find(g_effect.id);
    break;
  }
  case GameEffect::EffectSubType::kSkill: {
    node = WzResource::skill_->find(g_effect.id);
    break;
  }
  }
  auto index = std::to_string(g_effect.index);
  auto texture_node = node->get_child(index);
  uint32_t delay = 120;
  if (texture_node->get_child(u"delay")) {
    delay =
        static_cast<wz::Property<int32_t> *>(texture_node->get_child(u"delay"))
            ->get();
  }
  if (g_effect.time > delay) {
    g_effect.index += 1;
    g_effect.time = 0;
    if (g_effect.index >= node->children_count()) {
      r = true;
    }
  }
  return r;
}

bool EffectLogicSystem::RunSkillUse(GameEffect &g_effect) {
  bool r = false;
  auto ski_node = SkillGameInstance::LoadSkiNode(g_effect.id);
  if (g_effect.order.has_value()) {
    ski_node =
        ski_node->get_child("effect" + std::to_string(g_effect.order.value()));
  } else {
    ski_node = ski_node->get_child(u"effect");
  }
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto delay_node = texture_node->get_child(u"delay");
  int32_t delay = 100;
  if (delay_node) {
    delay = static_cast<wz::Property<std::int32_t> *>(delay_node)->get();
  }
  auto g_character = std::any_cast<GameCharacter *>(g_effect.data);
  auto action_type = CharacterLogicSystem::LoadActionType(*g_character);
  float speed = 1;
  switch (action_type) {
  case CharacterLogicSystem::ActionEnum::kAttack:
  case CharacterLogicSystem::ActionEnum::kSkill: {
    speed = CharacterLogicSystem::LoadAttackSpeed(*g_character);
    break;
  }
  default: {
    break;
  }
  }
  g_effect.time += Window::delta_time_ * speed;
  if (g_effect.time >= delay) {
    g_effect.time = 0;
    g_effect.index += 1;
    auto keys = ski_node->children;
    keys.erase(u"z");
    auto count = keys.size();
    r = g_effect.index >= count;
    g_effect.index = g_effect.index % count;
  }
  return r;
}

bool EffectLogicSystem::RunSkillAffected(GameEffect &g_effect) {
  bool r = false;
  auto ski_node = SkillGameInstance::LoadSkiNode(g_effect.id);
  ski_node = ski_node->get_child(u"affected");
  if (ski_node == nullptr) {
    return true;
  }
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto delay_node = texture_node->get_child(u"delay");
  int32_t delay = 100;
  if (delay_node) {
    delay = static_cast<wz::Property<std::int32_t> *>(delay_node)->get();
  }
  g_effect.time += Window::delta_time_;
  if (g_effect.time >= delay) {
    g_effect.time = 0;
    g_effect.index += 1;
    auto count = ski_node->children_count();
    r = g_effect.index >= count;
    g_effect.index = g_effect.index % count;
  }
  return r;
}

bool EffectLogicSystem::RunSkillHit(GameEffect &g_effect) {
  auto now = Window::dt_time_;
  if (g_effect.delay >= now) {
    return false;
  }
  bool r = false;
  auto ski_node = SkillGameInstance::LoadSkiNode(g_effect.id);
  if (ski_node->find(u"hit/0")) {
    ski_node = ski_node->find(u"hit/0");
  } else {
    ski_node =
        SkillGameInstance::LoadSkiLevelNode(g_effect.id, g_effect.lv);
    ski_node = ski_node->find(u"hit/0");
  }
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto delay_node = texture_node->get_child(u"delay");
  int delay = 100;
  if (delay_node) {
    delay = static_cast<wz::Property<int> *>(delay_node)->get();
  }
  g_effect.time += Window::delta_time_;
  if (g_effect.time >= delay) {
    g_effect.time = 0;
    g_effect.index += 1;
    auto count = ski_node->children_count();
    r = g_effect.index >= count;
    g_effect.index = g_effect.index % count;
  }
  return r;
}

bool EffectLogicSystem::RunSkillRegion(GameEffect &g_effect) {
  auto rski = std::any_cast<GameRegionSkill>(&g_effect.data);
  if (rski->end <= Window::dt_time_) {
    return true;
  }
  return false;
}

void EffectLogicSystem::RunAnimate(std::vector<GameEffect> &v) {
  for (auto it = v.begin(); it != v.end();) {
    bool remove = false;
    auto &g_effect = *it;
    switch (g_effect.type) {
    case GameEffect::EffectType::kEffect: {
      remove = RunEffect(g_effect);
      break;
    }
    case GameEffect::EffectType::kSkillUse: {
      remove = RunSkillUse(g_effect);
      break;
    }
    case GameEffect::EffectType::kSkillAffected: {
      remove = RunSkillAffected(g_effect);
      break;
    }
    case GameEffect::EffectType::kSkillHit: {
      remove = RunSkillHit(g_effect);
      break;
    }
    case GameEffect::EffectType::kDamage: {
      remove = RunDamage(g_effect);
      break;
    }
    case GameEffect::EffectType::kSkillRegion: {
      remove = RunSkillRegion(g_effect);
      break;
    }
    default: {
      break;
    }
    }

    if (remove) {       // 需要删除
      it = v.erase(it); // erase 返回下一个有效的迭代器
    } else {
      ++it; // 只有在不删除时才前进
    }
  }
}

bool EffectLogicSystem::Run() {
  RunAnimate(CharacterGameInstance::self_.effect);
  for (auto &o : CharacterGameInstance::others_ | std::views::values) {
    RunAnimate(o.g_character.effect);
  }
  for (auto &m : MobGameInstance::data_ | std::views::values) {
    RunAnimate(m.mob.effect);
  }
  for (auto &de : EffectGameInstance::data_) {
    RunAnimate(de);
  }
  return true;
}