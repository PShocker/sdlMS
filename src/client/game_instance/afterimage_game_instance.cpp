#include "src/client/game_instance/afterimage_game_instance.h"

#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
namespace sdlms {


std::u16string AfterimageGameInstance::LoadHitType(GameCharacter& g_character) {
  auto WeaponType = EquipGameInstance::LoadWeaponType(g_character);
  const static std::flat_map<EquipGameInstance::WeaponType, std::u16string> h =
      {
          {EquipGameInstance::WeaponType::kS1A1M1D, u"sword1"},
          {EquipGameInstance::WeaponType::kSpear, u"sword1"},
          {EquipGameInstance::WeaponType::kBow, u"sword1"},
          {EquipGameInstance::WeaponType::kCrossbow, u"sword1"},
          {EquipGameInstance::WeaponType::kS2A2M2, u"sword1"},
          {EquipGameInstance::WeaponType::kWand, u"sword1"},
          {EquipGameInstance::WeaponType::kClaw, u"sword1"},
          {EquipGameInstance::WeaponType::kGun, u"sword1"},
  };
  return h.at(WeaponType);
}

wz::Node* AfterimageGameInstance::LoadAtfNodeCache(const std::u16string& i) {
  static std::flat_map<std::u16string, wz::Node*> cache;
  if (!cache.contains(i)) {
    cache[i] = WzResource::character_->find(u"Afterimage/" + i + u".img");
  }
  return cache.at(i);
}

wz::Node* AfterimageGameInstance::LoadAtfNode(GameCharacter& g_character) {
  if (!g_character.weapon.has_value()) {
    return nullptr;
  }
  auto action = g_character.action;
  auto weapon_info =
      EquipGameInstance::LoadEquipInfo(g_character.weapon.value().id);
  auto weapon_level = weapon_info->get_child(u"reqLevel");
  auto weapon_level2 = static_cast<wz::Property<int>*>(weapon_level)->get();
  auto aft_type = weapon_info->get_child(u"afterImage");
  auto aft_type2 = static_cast<wz::Property<std::u16string>*>(aft_type)->get();
  auto aft_node = LoadAtfNodeCache(aft_type2);
  auto weapon_level3 = std::to_string(weapon_level2 / 10);
  aft_node = aft_node->get_child(weapon_level3);
  if (auto r = aft_node->get_child(action)) {
    return r;
  }
  return nullptr;
}

std::optional<AfterimageData> AfterimageGameInstance::LoadData(
    GameCharacter& g_character) {
  if (auto data = LoadAtfNode(g_character)) {
    int32_t action_i = 0;
    wz::Node* n;
    for (auto [k, v] : *data->get_children()) {
      if (!(k == u"lt" || k == u"rb")) {
        n = v[0];
        action_i = std::stoi(std::string{k.begin(), k.end()});
        break;
      }
    }
    if (action_i > g_character.action_index) {
      return std::nullopt;
    }
    int32_t character_time = g_character.action_time;
    for (uint8_t i = action_i; i < g_character.action_index; i++) {
      auto delay =
          CharacterGameInstance::bone_data_[g_character.action][i].delay;
      character_time += delay;
    }
    // 根据总时间判断是哪一帧
    int32_t afterimage_time = 0;
    for (auto [k, v] : *n->get_children()) {
      // std::flat是有序容器,所以遍历是顺序的
      auto delay =
          static_cast<wz::Property<int>*>(v[0]->get_child(u"delay"))->get();
      afterimage_time += delay;
      if (afterimage_time >= character_time) {
        AfterimageData r;
        // 判断帧内时间
        auto delta = delay - (afterimage_time - character_time);
        r.texture = WzResource::LoadTexture(v[0]);
        r.origin = WzResource::LoadFpoint(v[0]->get_child(u"origin"));
        int32_t a0 = 255;
        int32_t a1;
        if (v[0]->get_child(u"a1")) {
          a1 = static_cast<wz::Property<int>*>(v[0]->get_child(u"a1"))->get();
        } else {
          a1 = 255;
        }
        float t = (float)delta / (float)delay;
        r.alpha = a0 + (a1 - a0) * t;
        return r;
      }
    }
  }
  return std::nullopt;
}

std::optional<SDL_FRect> AfterimageGameInstance::LoadRect(
    GameCharacter& g_character) {
  if (auto data = LoadAtfNode(g_character)) {
    auto lt = WzResource::LoadFpoint(data->get_child(u"lt"));
    auto rb = WzResource::LoadFpoint(data->get_child(u"rb"));
    SDL_FRect rect{
        .x = lt.x,
        .y = lt.y,
        .w = rb.x - lt.x,
        .h = rb.y - lt.y,
    };
    rect.x += g_character.pos.x;
    rect.y += g_character.pos.y;
    if (g_character.flip == 1) {
      rect.x += 2 * (g_character.pos.x - rect.x) - rect.w;
    }
    return rect;
  }
  return std::nullopt;
}

uint64_t AfterimageGameInstance::LoadBeatTime(GameCharacter& g_character) {
  uint64_t time = 0;
  auto speed = CharacterLogicSystem::LoadAttackSpeed(g_character);
  speed = 1.0 / speed;
  if (auto data = LoadAtfNode(g_character)) {
    auto childs = *data->get_children();
    childs.erase(u"lt");
    childs.erase(u"rb");
    auto it = childs.begin();
    const auto& key = it->first;
    auto index = std::stoi(std::string{key.begin(), key.end()});
    const auto& bones =
        CharacterGameInstance::bone_data_.at(g_character.action);
    for (auto i = 0; i < index; i++) {
      time += bones[i].delay * speed;
    }
    auto now = Window::dt_time_;
    time += now;
  }
  return time;
}

}  // namespace sdlms
