#include "character_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "effect_game_instance.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_nametag.h"
#include "src/client/game/game_portal.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <algorithm>
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

void character_game_instance::load_self() {
  const auto &pos = scene_system_instance::prepare_pos;
  if (pos.has_value()) {
    self.pos = pos.value();
    self.action = u"jump";
    self.action_index = 0;
    self.action_time = 0;
  }
  character_logic_system::self_fh = 0;
  character_logic_system::self_lr = 0;

  self.tomb = std::nullopt;
}

SDL_FPoint character_game_instance::load_self_pos(const std::u16string &pn,
                                                  uint8_t index) {
  SDL_FPoint r;
  game_portal *por;
  auto portals =
      portal_game_instance::load(scene_system_instance::prepare_map_id);
  if (index == 0) {
    // 只需要第一个元素，使用 find 更高效
    auto it = portals.find(pn);
    if (it != portals.end()) {
      por = &it->second;
    }
  } else {
    // 需要第 N 个元素（N>0）
    auto [first, last] = portals.equal_range(pn);
    if (std::distance(first, last) > index) {
      auto it = std::ranges::next(first, index);
      por = &it->second;
    }
  }
  r = por->pos;
  r.y -= 5;
  return r;
}

void character_game_instance::load_self_nametag() {
  game_nametag nametag;
  nametag.text = u"进击的蓝蘑菇";
  nametag.path = u"14";
  nametag.pos = {0, 0};
  nametag.size = 13;
  nametag.color = {255, 255, 255, 255};
  self.nametags.push_back(nametag);
}

void character_game_instance::load_self_character() {
  add_body(self, u"00002000");
  add_head(self, u"00012000");
  add_coat(self, u"01040002");
  add_cap(self, u"01002005");
  add_weapon(self, u"01372006");
  add_pants(self, u"01060001");
  add_face(self, u"00020000");
  add_hair(self, u"00030000");
  add_shield(self, u"01092003");
  load_self_nametag();
}

void character_game_instance::load_others_character(
    const std::unique_ptr<fbs::PlayerT> &c) {

  game_character g_character;
  const auto &appearance = c->character->appearance;

  std::string tmp = std::format("{:08d}", appearance->body);
  g_character.body = {tmp.begin(), tmp.end()};

  tmp = std::format("{:08d}", appearance->head);
  g_character.head = {tmp.begin(), tmp.end()};

  tmp = std::format("{:08d}", appearance->face);
  g_character.face.id = {tmp.begin(), tmp.end()};

  tmp = std::format("{:08d}", appearance->hair);
  g_character.hair = {tmp.begin(), tmp.end()};

  if (appearance->weapon != 0) {
    auto &weapon = g_character.weapon.emplace();
    tmp = std::format("{:08d}", appearance->weapon);
    weapon.id = {tmp.begin(), tmp.end()};
  }
  if (appearance->cap != 0) {
    auto &cap = g_character.cap.emplace();
    tmp = std::format("{:08d}", appearance->cap);
    cap.id = {tmp.begin(), tmp.end()};
  }
  if (appearance->cape != 0) {
    auto &cape = g_character.cape.emplace();
    tmp = std::format("{:08d}", appearance->cape);
    cape.id = {tmp.begin(), tmp.end()};
  }
  if (appearance->coat != 0) {
    auto &coat = g_character.coat.emplace();
    tmp = std::format("{:08d}", appearance->coat);
    coat.id = {tmp.begin(), tmp.end()};
  }
  if (appearance->glove != 0) {
    auto &glove = g_character.glove.emplace();
    tmp = std::format("{:08d}", appearance->glove);
    glove.id = {tmp.begin(), tmp.end()};
  }
  if (appearance->pant != 0) {
    auto &pant = g_character.pant.emplace();
    tmp = std::format("{:08d}", appearance->pant);
    pant.id = {tmp.begin(), tmp.end()};
  }
  if (appearance->shield != 0) {
    auto &shield = g_character.shield.emplace();
    tmp = std::format("{:08d}", appearance->shield);
    shield.id = {tmp.begin(), tmp.end()};
  }
  if (appearance->longcoat != 0) {
    auto &longcoat = g_character.longcoat.emplace();
    tmp = std::format("{:08d}", appearance->longcoat);
    longcoat.id = {tmp.begin(), tmp.end()};
  }
  if (appearance->shoes != 0) {
    auto &shoes = g_character.shoes.emplace();
    tmp = std::format("{:08d}", appearance->shoes);
    shoes.id = {tmp.begin(), tmp.end()};
  }
  if (appearance->accessory != 0) {
    auto &accessory = g_character.accessory.emplace();
    tmp = std::format("{:08d}", appearance->accessory);
    accessory.id = {tmp.begin(), tmp.end()};
  }
  const auto &state = c->character->state;
  g_character.pos = SDL_FPoint{
      state->x,
      state->y,
  };
  g_character.action =
      std::u16string{state->action.begin(), state->action.end()};
  if (g_character.action == u"dead") {
    game_tomb t{
        .ani_type = u"land",
        .ani_index = 0,
        .ani_time = 0,
        .pos = {g_character.pos.x, g_character.pos.y},
        .b = g_character.pos,
    };
    g_character.tomb = t;
  }
  g_character.action_index = state->action_index;
  g_character.action_time = 0;
  g_character.action_animate = c->character->state->action_animate;
  g_character.page = c->character->state->page;
  g_character.flip = c->character->state->flip;
  game_nametag nametag;
  nametag.path = u"";
  nametag.pos = {0, 0};
  nametag.size = 13;
  nametag.color = {255, 255, 255, 255};
  nametag.text =
      std::u16string{c->character->name.begin(), c->character->name.end()};

  g_character.nametags.push_back(nametag);

  g_character.fame = c->character->fame;
  g_character.level = c->character->level;
  g_character.job =
      std::u16string{c->character->job.begin(), c->character->job.end()};
  g_character.face.action = std::u16string{c->character->face_action.begin(),
                                           c->character->face_action.end()};
  others.emplace(c->client_id, g_character);
}

void character_game_instance::clear_others() { others.clear(); }

void character_game_instance::exit_others_character(uint64_t client_id) {
  others.erase(client_id);
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
      if (k == u"info") {
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
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

void character_game_instance::add_coat(game_character &g,
                                       const std::u16string &val) {
  game_equip g_equip;
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

void character_game_instance::add_cap(game_character &g,
                                      const std::u16string &val) {
  game_equip g_equip;
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

void character_game_instance::add_pants(game_character &g,
                                        const std::u16string &val) {
  game_equip g_equip;
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

void character_game_instance::add_weapon(game_character &g,
                                         const std::u16string &val) {
  game_equip g_equip;
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

void character_game_instance::add_shield(game_character &g,
                                         const std::u16string &val) {
  game_equip g_equip;
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

void character_game_instance::add_cape(game_character &g,
                                       const std::u16string &val) {
  game_equip g_equip;
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

void character_game_instance::add_accessory(game_character &g,
                                            const std::u16string &val) {
  game_equip g_equip;
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

void character_game_instance::add_glove(game_character &g,
                                        const std::u16string &val) {
  game_equip g_equip;
  g_equip.id = val;
  g.glove = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto glove_node = character_node->find(u"Cape/" + val + u".img");
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

void character_game_instance::add_longcoat(game_character &g,
                                           const std::u16string &val) {
  game_equip g_equip;
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

void character_game_instance::add_shoes(game_character &g,
                                        const std::u16string &val) {
  game_equip g_equip;
  g_equip.id = val;
  g.shoes = g_equip;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto shoes_node = character_node->find(u"Longcoat/" + val + u".img");
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
            c.z =
                static_cast<wz::Property<std::u16string> *>(f_node->find(u"z"))
                    ->get();
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

fbs::CharacterT
character_game_instance::load_characterT(const game_character &g) {
  fbs::CharacterT c;

  c.appearance = std::make_unique<fbs::CharacterAppearanceT>();
  c.state = std::make_unique<fbs::LifeStateT>();

  if (self.accessory.has_value()) {
    auto id = g.accessory->id;
    c.appearance->accessory = std::stoi(std::string{id.begin(), id.end()});
  }
  if (self.cap.has_value()) {
    auto id = g.cap->id;
    c.appearance->cap = std::stoi(std::string{id.begin(), id.end()});
  }
  if (self.cape.has_value()) {
    auto id = g.cape->id;
    c.appearance->cape = std::stoi(std::string{id.begin(), id.end()});
  }
  if (self.glove.has_value()) {
    auto id = g.glove->id;
    c.appearance->glove = std::stoi(std::string{id.begin(), id.end()});
  }
  if (self.coat.has_value()) {
    auto id = g.coat->id;
    c.appearance->coat = std::stoi(std::string{id.begin(), id.end()});
  }
  if (self.longcoat.has_value()) {
    auto id = g.longcoat->id;
    c.appearance->longcoat = std::stoi(std::string{id.begin(), id.end()});
  }
  if (self.pant.has_value()) {
    auto id = g.pant->id;
    c.appearance->pant = std::stoi(std::string{id.begin(), id.end()});
  }
  if (self.shield.has_value()) {
    auto id = g.shield->id;
    c.appearance->shield = std::stoi(std::string{id.begin(), id.end()});
  }
  if (self.weapon.has_value()) {
    auto id = g.weapon->id;
    c.appearance->weapon = std::stoi(std::string{id.begin(), id.end()});
  }
  if (self.shoes.has_value()) {
    auto id = g.shoes->id;
    c.appearance->shoes = std::stoi(std::string{id.begin(), id.end()});
  }
  c.appearance->head = std::stoi(std::string{g.head.begin(), g.head.end()});
  c.appearance->body = std::stoi(std::string{g.body.begin(), g.body.end()});

  c.appearance->hair = std::stoi(std::string{g.hair.begin(), g.hair.end()});

  c.appearance->face =
      std::stoi(std::string{g.face.id.begin(), g.face.id.end()});

  c.state->action = std::string{g.action.begin(), g.action.end()};
  c.state->x = g.pos.x;
  c.state->y = g.pos.y;

  c.state->flip = g.flip;

  auto name = g.nametags[0].text;
  c.name = std::vector<uint16_t>{name.begin(), name.end()};

  c.fame = g.fame;
  c.face_action = std::string{g.face.action.begin(), g.face.action.end()};
  c.job = std::string{g.job.begin(), g.job.end()};
  c.level = g.level;

  return c;
}

void character_game_instance::load_character_attack(
    const std::vector<std::unique_ptr<fbs::CharacterAttackT>> &v,
    game_character &g_character) {
  auto &mobs = mob_game_instance::data;
  std::flat_multiset<uint32_t> mob_hit;
  for (uint32_t i = 0; i < v.size(); i++) {
    auto &ct = v[i];
    auto &mob = mobs[ct->mob_index].mob;
    if (ct->afterimage) {
      game_effect e = {
          .id = afterimage_game_instance::load_hit_type(g_character),
          .index = 0,
          .time = 0,
          .delay = ct->attack->delay,
          .type = game_effect::effect_type::afterimage,
          .pos = SDL_FPoint{ct->attack->x, ct->attack->y},
          .z = false,
      };
      mob.effect.push_back(e);
    }
    // 伤害数字
    damage_data data = {
        .num = ct->attack->num,
        .type = damage_data::red,
    };
    game_effect d = {
        .id = u"",
        .index = (uint32_t)mob_hit.count(mob.index),
        .time = mob.index,
        .delay = ct->attack->delay,
        .type = game_effect::effect_type::damage,
        .pos = SDL_FPoint{ct->attack->x, ct->attack->y - 10},
        .z = false,
        .flip = false,
        .data = data,
    };
    mob_hit.insert(mob.index);
    effect_game_instance::data[7].emplace_back(d);
  }
}

void character_game_instance::load_character_skill(
    uint32_t ski_id,
    const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
    game_character &g_character) {
  auto ski_id2 = std::to_string(ski_id);
  auto ski_id3 = std::u16string{ski_id2.begin(), ski_id2.end()};

  g_character.skill = ski_id3;

  game_effect e = {
      .id = ski_id3,
      .index = 0,
      .time = 0,
      .delay = 0,
      .type = game_effect::effect_type::skill_use,
      .pos = std::nullopt,
      .z = false,
  };
  g_character.effect.push_back(e);

  auto &mob = mob_game_instance::data;
  for (const auto &s : v) {
    game_effect e2 = {
        .id = ski_id3,
        .index = 0,
        .time = 0,
        .delay = s->delay,
        .type = game_effect::effect_type::skill_hit,
        .pos = SDL_FPoint{s->x, s->y},
        .z = false,
    };
    mob[s->mob].mob.effect.push_back(e2);
  }
}

void character_game_instance::other_character_logic(
    const fbs::ServerCharacterLogicT &r) {
  const auto client_id = r.payload->client_id;
  if (others.contains(client_id)) {
    auto &logics = others[client_id].logics;
    auto type = r.payload->payload.type;
    logics[type].push_back(r.payload->payload);
  }
}

void character_game_instance::other_character_attack(
    const ServerCharacterAttackT &r) {
  const auto client_id = r.client_id;
  if (others.contains(client_id)) {
    auto &g_character = others.at(client_id).g_character;
    character_game_instance::load_character_attack(r.payload, g_character);
  }
}

void character_game_instance::other_character_skill(
    const ServerCharacterSkillT &r) {
  const auto client_id = r.client_id;
  if (others.contains(client_id)) {
    auto &g = others.at(client_id).g_character;
    character_game_instance::load_character_skill(r.ski_id, r.payload, g);
  }
}