#include "character_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "equip_game_instance.h"
#include "src/client/game/game_character.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

void character_game_instance::init_character_bone() {
  auto character_node = wz_resource::character;

  auto body_node = character_node->find(u"00002000.img");
  auto head_node = character_node->find(u"00012000.img");

  struct tmp_bone {
    std::u16string name;
    wz::Node *node;
  };
  struct tmp_bone_data {
    // z node
    std::flat_multimap<std::u16string, tmp_bone> bones;
  };
  // action index
  std::flat_map<std::u16string, std::vector<tmp_bone_data>> tmp_bone_map;
  for (auto [key, val] : *body_node->get_children()) {
    if (key == u"info") {
      continue;
    }
    auto action_node = val[0];

    for (uint8_t frame = 0; frame < action_node->children_count(); frame++) {
      auto format2 = std::to_string(frame);
      auto body_frame_node = action_node->get_child(format2);
      if (body_frame_node->get_child(u"action") == nullptr) {

        tmp_bone_map[key].resize(action_node->children_count());
        bone_data[key].resize(action_node->children_count());

        if (body_frame_node->get_child(u"delay")) {
          auto delay = static_cast<wz::Property<int> *>(
                           body_frame_node->get_child(u"delay"))
                           ->get();
          bone_data[key][frame].delay = delay;
        }

        auto face = static_cast<wz::Property<int16_t> *>(
                        body_frame_node->get_child(u"face"))
                        ->get();
        bone_data[key][frame].face = face;

        //  基础动作，身体
        auto format3 = std::u16string{format2.begin(), format2.end()};
        auto head_frame_node = head_node->find(key + u"/" + format3);
        if (head_frame_node == nullptr) {
          // 默认default
          head_frame_node = head_node->find(u"front");
        }

        for (auto parts : {*body_frame_node->get_children(),
                           *head_frame_node->get_children()}) {
          for (auto [k, v] : parts) {
            auto part_node = v[0];
            if (part_node->type == wz::Type::UOL) {
              part_node =
                  static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
            }
            if (!part_node->get_child(u"z")) {
              continue;
            }
            auto z = static_cast<wz::Property<std::u16string> *>(
                         part_node->get_child(u"z"))
                         ->get();
            auto map_node = part_node->get_child(u"map");
            tmp_bone_map[key][frame].bones.insert({z, {k, part_node}});
          }
        }
      } else {
        character_extern_action e_action;
        // 引用动作
        e_action.action = static_cast<wz::Property<std::u16string> *>(
                              body_frame_node->get_child(u"action"))
                              ->get();
        e_action.frame = static_cast<wz::Property<int> *>(
                             body_frame_node->get_child(u"frame"))
                             ->get();
        e_action.delay = static_cast<wz::Property<int> *>(
                             body_frame_node->get_child(u"delay"))
                             ->get();
        SDL_FPoint move = {0, 0};
        if (body_frame_node->get_child(u"move")) {
          auto v = static_cast<wz::Property<wz::WzVec2D> *>(
                       body_frame_node->get_child(u"move"))
                       ->get();
          e_action.move = {static_cast<float>(v.x), static_cast<float>(v.y)};
        }
        extern_action[key].push_back(e_action);
      }
    }
  }
  for (const auto [k, v] : tmp_bone_map) {
    for (uint8_t i = 0; i < v.size(); i++) {
      const auto &tbd = v[i];
      for (const auto &zp : zmap) {
        if (!tbd.bones.contains(zp)) {
          continue;
        }
        // const auto &bones = tbd.bones.at(zp);
        auto [beg, end] = tbd.bones.equal_range(zp);
        for (auto it = beg; it != end; ++it) {
          auto &bones = it->second;
          // 构建骨骼
          std::u16string parent_bone_name = u"";
          for (auto [pk, pv] : *bones.node->get_child(u"map")->get_children()) {
            if (bone_data[k][i].bone_pos.contains(pk)) {
              parent_bone_name = pk;
            }
          }
          if (parent_bone_name == u"") {
            // 无父节点
            for (auto [pk, pv] :
                 *bones.node->get_child(u"map")->get_children()) {
              auto vec = static_cast<wz::Property<wz::WzVec2D> *>(pv[0])->get();
              bone_data[k][i].bone_pos[pk] = {float(vec.x), float(vec.y)};
            }
            bone_data[k][i].part_pos[bones.name] = {0, 0};
          } else {
            // 有父节点
            auto parent_bone_pos =
                bone_data[k][i].bone_pos.at(parent_bone_name);
            auto parent_vec = bones.node->find(u"map/" + parent_bone_name);
            auto vec =
                static_cast<wz::Property<wz::WzVec2D> *>(parent_vec)->get();
            SDL_FPoint cur_pos{parent_bone_pos.x - vec.x,
                               parent_bone_pos.y - vec.y};
            bone_data[k][i].part_pos[bones.name] = cur_pos;
            for (auto [pk, pv] :
                 *bones.node->get_child(u"map")->get_children()) {
              if (bone_data[k][i].bone_pos.contains(pk)) {
                continue;
              }
              auto vec = static_cast<wz::Property<wz::WzVec2D> *>(pv[0])->get();
              bone_data[k][i].bone_pos[pk] = {float(cur_pos.x + vec.x),
                                              float(cur_pos.y + vec.y)};
            }
          }
        }
      }
    }
  }
}

void character_game_instance::init_default_clothes() {
  game_character g;
  add_coat(g, u"01040000");
  add_pants(g, u"01060000");
  return;
}

void character_game_instance::load_self_character() {
  add_body(self, u"00002000");
  add_head(self, u"00012000");
  add_coat(self, u"01040002");
  add_cap(self, u"01002005");
  add_weapon(self, u"01472012");
  add_pants(self, u"01060001");
  add_face(self, u"00020000");
  add_hair(self, u"00030000");
  add_shield(self, u"01092003");
  add_ear(self, u"humanEar");

  add_cap_deco(self, u"01007002");
  add_weapon_deco(self, u"01702017");
}

std::flat_set<std::u16string>
character_game_instance::split_vslot(const std::u16string &vslot) {
  std::flat_set<std::u16string> result;
  for (size_t i = 0; i < vslot.size(); i += 2) {
    result.insert(vslot.substr(i, 2));
  }
  return result;
}

std::u16string
character_game_instance::split_islot(const std::u16string &islot) {
  auto result = split_vslot(islot);
  for (auto k : zmap2) {
    if (result.contains(k)) {
      return k;
    }
  }
  return islot;
}

void character_game_instance::add_body(game_character &g,
                                       const std::u16string &val) {
  g.body = val;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto body_node = character_node->find(val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  body_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              body_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *body_node->get_children()) {
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            c.pos = bone_data[k][frame].part_pos.at(bk);
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_head(game_character &g,
                                       const std::u16string &val) {
  g.head = val;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto head_node = character_node->find(val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  head_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              head_node->find(u"info/vslot"))
                              ->get());
    auto child = *head_node->get_children();
    child.erase(u"info");
    child[u"dead"] = {child[u"jump"].at(0)};
    for (auto [k, v] : child) {
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            c.pos = bone_data[k][frame].part_pos.at(bk);
            c.name = bk;
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_ear(game_character &g,
                                      const std::u16string &val) {
  g.ear = val;
  std::flat_set<std::u16string> hide_ear = {
      u"humanEar",
      u"ear",
      u"lefEar",
      u"highlefEar",
  };
  // 从 hide_part 中移除所有耳朵
  for (const auto &ear : hide_ear) {
    g.hide_part.erase(ear);
  }
  hide_ear.erase(val);
  g.hide_part.insert_range(hide_ear);
}

void character_game_instance::add_coat(game_character &g,
                                       const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.coat = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto coat_node = character_node->find(u"Coat/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  coat_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              coat_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *coat_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_coat_deco(game_character &g,
                                            const std::u16string &val) {
  auto coat = g.coat;
  add_coat(g, val);
  g.coat = coat;

  game_deco_item g_deco;
  g_deco.id = val;
  g.coat_deco = g_deco;
}

void character_game_instance::add_cap(game_character &g,
                                      const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.cap = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto cap_node = character_node->find(u"Cap/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  cap_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              cap_node->find(u"info/vslot"))
                              ->get());
    auto child = *cap_node->get_children();
    child.erase(u"info");
    child[u"dead"] = {child[u"jump"].at(0)};
    for (auto [k, v] : child) {
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_cap_deco(game_character &g,
                                           const std::u16string &val) {
  auto cap = g.cap;
  add_cap(g, val);
  g.cap = cap;

  game_deco_item g_deco;
  g_deco.id = val;
  g.cap_deco = g_deco;
}

void character_game_instance::add_pants(game_character &g,
                                        const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.pant = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto pant_node = character_node->find(u"Pants/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  pant_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              pant_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *pant_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_pants_deco(game_character &g,
                                             const std::u16string &val) {
  auto pant = g.pant;
  add_pants(g, val);
  g.pant = pant;

  game_deco_item g_deco;
  g_deco.id = val;
  g.pant_deco = g_deco;
}

void character_game_instance::add_weapon(game_character &g,
                                         const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.weapon = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto weapon_node = character_node->find(u"Weapon/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  weapon_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              weapon_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *weapon_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_weapon_deco(game_character &g,
                                              const std::u16string &val) {
  if (!g.weapon.has_value()) {
    return;
  }
  auto weapon_enum = g.weapon->id;
  std::u16string sub = weapon_enum.substr(2, 2);
  game_deco_item g_deco;
  g_deco.id = val;
  g.weapon_deco = g_deco;
  std::u16string deco_val = val + u"/" + sub;
  if (!avatar_data.contains(deco_val)) {
    character_avatar_render &r = avatar_data[deco_val];
    auto character_node = wz_resource::character;
    auto weapon_node = character_node->find(u"Weapon/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  weapon_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              weapon_node->find(u"info/vslot"))
                              ->get());
    weapon_node = weapon_node->get_child(sub);
    for (auto [k, v] : *weapon_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_shield(game_character &g,
                                         const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.shield = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto shield_node = character_node->find(u"Shield/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  shield_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              shield_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *shield_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_shield_deco(game_character &g,
                                              const std::u16string &val) {
  auto shield = g.shield;
  add_shield(g, val);
  g.shield = shield;

  game_deco_item g_deco;
  g_deco.id = val;
  g.shield_deco = g_deco;
}

void character_game_instance::add_cape(game_character &g,
                                       const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.cape = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto cape_node = character_node->find(u"Cape/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  cape_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              cape_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *cape_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_cape_deco(game_character &g,
                                            const std::u16string &val) {
  auto cape = g.cape;
  add_cape(g, val);
  g.cape = cape;

  game_deco_item g_deco;
  g_deco.id = val;
  g.cape_deco = g_deco;
}

void character_game_instance::add_accessory(game_character &g,
                                            const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.accessory = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto accessory_node = character_node->find(u"Cape/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  accessory_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              accessory_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *accessory_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_accessory_deco(game_character &g,
                                                 const std::u16string &val) {
  auto accessory = g.accessory;
  add_accessory(g, val);
  g.accessory = accessory;

  game_deco_item g_deco;
  g_deco.id = val;
  g.accessory_deco = g_deco;
}

void character_game_instance::add_glove(game_character &g,
                                        const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.glove = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto glove_node = character_node->find(u"Glove/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  glove_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              glove_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *glove_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        if (body_frame_node == nullptr) {
          continue;
        }
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_glove_deco(game_character &g,
                                             const std::u16string &val) {
  auto glove = g.glove;
  add_glove(g, val);
  g.glove = glove;

  game_deco_item g_deco;
  g_deco.id = val;
  g.glove_deco = g_deco;
}

void character_game_instance::add_longcoat(game_character &g,
                                           const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.longcoat = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto longcoat_node = character_node->find(u"Longcoat/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  longcoat_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              longcoat_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *longcoat_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_longcoat_deco(game_character &g,
                                                const std::u16string &val) {
  auto longcoat = g.longcoat;
  add_longcoat(g, val);
  g.longcoat = longcoat;

  game_deco_item g_deco;
  g_deco.id = val;
  g.longcoat_deco = g_deco;
}

void character_game_instance::add_shoes(game_character &g,
                                        const std::u16string &val) {
  game_equip_item g_equip;
  g_equip.id = val;
  g.shoes = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto shoes_node = character_node->find(u"Shoes/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  shoes_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              shoes_node->find(u"info/vslot"))
                              ->get());
    for (auto [k, v] : *shoes_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_shoes_deco(game_character &g,
                                             const std::u16string &val) {
  auto shoes = g.shoes;
  add_shoes(g, val);
  g.shoes = shoes;

  game_deco_item g_deco;
  g_deco.id = val;
  g.shoes_deco = g_deco;
}

void character_game_instance::add_hair(game_character &g,
                                       const std::u16string &val) {
  g.hair = val;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto hair_node = character_node->find(u"Hair/" + val + u".img");
    r.islot = static_cast<wz::Property<std::u16string> *>(
                  hair_node->find(u"info/islot"))
                  ->get();
    r.islot = split_islot(r.islot);
    r.vslot = split_vslot(static_cast<wz::Property<std::u16string> *>(
                              hair_node->find(u"info/vslot"))
                              ->get());
    auto child = *hair_node->get_children();
    child.erase(u"info");
    child[u"dead"] = {child[u"jump"].at(0)};
    for (auto [k, v] : child) {
      if (!bone_data.contains(k)) {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [bk, bv] : *body_frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            character_avatar c;
            c.texture = wz_resource::load_texture(part_node);
            c.z = static_cast<wz::Property<std::u16string> *>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D> *>(part_val)->get();
            auto parent_pos = bone_data[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_face(game_character &g,
                                       const std::u16string &val) {
  game_face g_face;
  g_face.id = val;
  g.face = g_face;
  if (!face_data.contains(val)) {
    auto character_node = wz_resource::character;
    auto face_node = character_node->find(u"Face/" + val + u".img");
    for (auto [k, v] : *face_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      auto &f = face_data[val];
      for (const auto &[bk, bv] : bone_data) {
        for (const auto &cbd : bv) {
          if (!cbd.bone_pos.contains(u"brow")) {
            continue;
          }
          auto brow_pos = cbd.bone_pos.at(u"brow");
          std::vector<character_avatar> vc;
          std::vector<uint32_t> vd;
          for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
            auto format2 = std::to_string(frame);
            auto f_node = v[0]->get_child(format2);
            if (frame == 0 && v[0]->get_child(u"face")) {
              // default
              f_node = v[0]->get_child(u"face");
            } else if (f_node == nullptr) {
              break;
            } else {
              f_node = f_node->get_child(u"face");
            }
            character_avatar c;
            auto f_brow = static_cast<wz::Property<wz::WzVec2D> *>(
                              f_node->find(u"map/brow"))
                              ->get();
            c.texture = wz_resource::load_texture(f_node);
            // 直接写face
            c.z = u"face";
            auto ori = static_cast<wz::Property<wz::WzVec2D> *>(
                           f_node->find(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            c.pos = {brow_pos.x - f_brow.x, brow_pos.y - f_brow.y};
            vc.push_back(c);

            // delay
            uint32_t delay = 0;
            if (f_node->find(u"../delay")) {
              delay =
                  static_cast<wz::Property<int> *>(f_node->find(u"../delay"))
                      ->get();
            }
            vd.push_back(delay);
          }
          f.delay[k] = vd;
          f.data[k].data[bk].push_back(vc);
          f.data[k].islot = u"Fc";
          f.data[k].vslot = {u"Fc"}; // face只占用Fc
        }
      }
    }
  }
}

void character_game_instance::add_ring0_deco(game_character &g,
                                             const std::u16string &val) {
  game_deco_item g_deco;
  g_deco.id = val;
  g.ring0_deco = g_deco;
}

void character_game_instance::add_ring1_deco(game_character &g,
                                             const std::u16string &val) {
  game_deco_item g_deco;
  g_deco.id = val;
  g.ring1_deco = g_deco;
}