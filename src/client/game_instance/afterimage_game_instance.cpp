#include "afterimage_game_instance.h"
#include "character_game_instance.h"
#include "equip_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>

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

std::optional<afterimage_ltrb>
afterimage_game_instance::load_rect(game_character &g_character) {
  if (auto data = load_atf_node(g_character)) {
    auto action_i = std::to_string(g_character.action_index);
    if (data->get_child(action_i) && g_character.action_time == 0) {
      auto lt = wz_resource::load_fpoint(data->get_child(u"lt"));
      auto rb = wz_resource::load_fpoint(data->get_child(u"rb"));
      return afterimage_ltrb{lt, rb};
    }
  }
  return std::nullopt;
}