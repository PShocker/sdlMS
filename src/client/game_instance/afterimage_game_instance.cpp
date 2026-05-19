#include "afterimage_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "character_game_instance.h"
#include "effect_game_instance.h"
#include "equip_game_instance.h"
#include "src/client/game/game_effect.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>

std::u16string
afterimage_game_instance::load_hit_type(game_character &g_character) {
  auto g_weapon = g_character.weapon->id;
  auto g_weapon_info = equip_game_instance::load_equip_info(g_weapon);
  auto attack_type =
      static_cast<wz::Property<int16_t> *>(g_weapon_info->get_child(u"attack"))
          ->get();
  enum weapon_type : uint8_t {
    NONE = 0,
    S1A1M1D = 1,
    SPEAR = 2,
    BOW = 3,
    CROSSBOW = 4,
    S2A2M2 = 5,
    WAND = 6,
    CLAW = 7,
    GUN = 9,
  };
  const std::flat_map<weapon_type, std::u16string> h = {
      {S1A1M1D, u"sword1"},  {SPEAR, u"sword1"},  {BOW, u"sword1"},
      {CROSSBOW, u"sword1"}, {S2A2M2, u"sword1"}, {WAND, u"sword1"},
      {CLAW, u"sword1"},     {GUN, u"sword1"},
  };
  return h.at((weapon_type)attack_type);
}

wz::Node *
afterimage_game_instance::load_atf_node_cache(const std::u16string &i) {
  static std::flat_map<std::u16string, wz::Node *> cache;
  if (!cache.contains(i)) {
    cache[i] = wz_resource::character->find(u"Afterimage/" + i + u".img");
  }
  return cache.at(i);
}

wz::Node *afterimage_game_instance::load_atf_node(game_character &g_character) {
  if (!g_character.weapon.has_value()) {
    return nullptr;
  }
  auto action = g_character.action;
  auto weapon_info =
      equip_game_instance::load_equip_info(g_character.weapon.value().id);
  auto weapon_level = weapon_info->get_child(u"reqLevel");
  auto weapon_level2 = static_cast<wz::Property<int> *>(weapon_level)->get();
  auto aft_type = weapon_info->get_child(u"afterImage");
  auto aft_type2 = static_cast<wz::Property<std::u16string> *>(aft_type)->get();
  auto aft_node = load_atf_node_cache(aft_type2);
  auto weapon_level3 = std::to_string(weapon_level2 / 10);
  aft_node = aft_node->get_child(weapon_level3);
  if (auto r = aft_node->get_child(action)) {
    return r;
  }
  return nullptr;
}

std::optional<afterimage_data>
afterimage_game_instance::load_data(game_character &g_character) {
  if (auto data = load_atf_node(g_character)) {
    int32_t action_i = 0;
    wz::Node *n;
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
          character_game_instance::bone_data[g_character.action][i].delay;
      character_time += delay;
    }
    // 根据总时间判断是哪一帧
    int32_t afterimage_time = 0;
    for (auto [k, v] : *n->get_children()) {
      // std::flat是有序容器,所以遍历是顺序的
      auto delay =
          static_cast<wz::Property<int> *>(v[0]->get_child(u"delay"))->get();
      afterimage_time += delay;
      if (afterimage_time >= character_time) {
        afterimage_data r;
        // 判断帧内时间
        auto delta = delay - (afterimage_time - character_time);
        r.texture = wz_resource::load_texture(v[0]);
        r.origin = wz_resource::load_fpoint(v[0]->get_child(u"origin"));
        int32_t a0 = 255;
        int32_t a1;
        if (v[0]->get_child(u"a1")) {
          a1 = static_cast<wz::Property<int> *>(v[0]->get_child(u"a1"))->get();
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

std::optional<SDL_FRect>
afterimage_game_instance::load_rect(game_character &g_character) {
  if (auto data = load_atf_node(g_character)) {
    auto lt = wz_resource::load_fpoint(data->get_child(u"lt"));
    auto rb = wz_resource::load_fpoint(data->get_child(u"rb"));
    return SDL_FRect{
        .x = lt.x,
        .y = lt.y,
        .w = rb.x - lt.x,
        .h = rb.y - lt.y,
    };
  }
  return std::nullopt;
}

uint64_t afterimage_game_instance::load_beat_time(game_character &g_character) {
  uint64_t time = 0;
  if (auto data = load_atf_node(g_character)) {
    auto childs = *data->get_children();
    childs.erase(u"lt");
    childs.erase(u"rb");
    auto it = childs.begin();
    const auto &key = it->first;
    auto index = std::stoi(std::string{key.begin(), key.end()});
    const auto &bones =
        character_game_instance::bone_data.at(g_character.action);
    for (auto i = 0; i < index; i++) {
      time += bones[i].delay;
    }
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
    time += now;
  }
  return time;
}