#include "character_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

void character_game_instance::init_character_bone() {
  auto character_node = wz_resource::character;

  auto body_node = character_node->find(u"00002000.img");
  auto head_node = character_node->find(u"00012000.img");

  struct tmp_bone_data {
    std::flat_map<std::u16string, std::vector<wz::Node *>> bones;
  };

  std::flat_map<std::u16string, std::vector<tmp_bone_data>> tmp_bone_map;
  for (auto [key, val] : *body_node->get_children()) {
    if (key == u"info") {
      continue;
    }
    auto action_node = val[0];

    tmp_bone_map[key].resize(action_node->children_count());
    bone_data[key].resize(action_node->children_count());

    for (uint8_t frame = 0; frame < action_node->children_count(); frame++) {
      auto format2 = std::to_string(frame);
      auto body_frame_node = action_node->get_child(format2);
      if (body_frame_node->get_child(u"action") == nullptr) {
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
            tmp_bone_map[key][frame].bones[z].push_back(map_node);
          }
        }
      } else {
        // 引用动作
        auto action = static_cast<wz::Property<std::u16string> *>(
                          body_frame_node->get_child(u"action"))
                          ->get();
        auto frame = static_cast<wz::Property<int> *>(
                         body_frame_node->get_child(u"frame"))
                         ->get();
        auto delay = static_cast<wz::Property<int> *>(
                         body_frame_node->get_child(u"delay"))
                         ->get();
        SDL_FPoint move = {0, 0};
        if (body_frame_node->get_child(u"move")) {
          auto v = static_cast<wz::Property<wz::WzVec2D> *>(
                       body_frame_node->get_child(u"move"))
                       ->get();
          move = {static_cast<float>(v.x), static_cast<float>(v.y)};
        }
        character_extern_action e_action = {.action = action,
                                            .frame =
                                                static_cast<uint8_t>(frame),
                                            .delay = delay,
                                            .move = move};
        extern_action.insert({key, e_action});
      }
    }
  }
  for (const auto [k, v] : tmp_bone_map) {
    for (uint8_t i = 0; i < v.size(); i++) {
      const auto &tbd = v[i];
      for (const auto &v : zmap) {
        if (tbd.bones.contains(v)) {
          for (uint8_t m = 0; m < tbd.bones.at(v).size(); m++) {
            auto map_node = tbd.bones.at(v)[m];
            std::optional<std::u16string> parent_bone;
            std::optional<SDL_FPoint> parent_pos;
            // 先找到是否有父节点
            for (auto [key, val] : *map_node->get_children()) {
              auto v = static_cast<wz::Property<wz::WzVec2D> *>(val[0])->get();
              if (bone_data.contains(key)) {
                parent_bone = key;
                parent_pos = SDL_FPoint{static_cast<float>(v.x),
                                        static_cast<float>(v.y)};
                break;
              }
            }
            if (parent_bone.has_value()) {
              // 存在父节点
              auto pb = bone_data.at(k).at(i).bone_pos.at(parent_bone.value());
              SDL_FPoint cur_pos{pb.x - parent_pos.value().x,
                                 pb.y - parent_pos.value().y};
              for (auto [key, val] : *map_node->get_children()) {
                if (bone_data.contains(key)) {
                  continue;
                }
                auto v =
                    static_cast<wz::Property<wz::WzVec2D> *>(val[0])->get();
                bone_data[k][i].bone_pos[key] = SDL_FPoint{
                    cur_pos.x - v.x,
                    cur_pos.y - v.y,
                };
              }
            } else {
              // 没有父节点
              for (auto [key, val] : *map_node->get_children()) {
                auto v =
                    static_cast<wz::Property<wz::WzVec2D> *>(val[0])->get();
                bone_data[k][i].bone_pos[key] = SDL_FPoint{
                    static_cast<float>(v.x),
                    static_cast<float>(v.y),
                };
              }
            }
          }
        }
      }
    }
  }
}

void character_game_instance::load_self_character() {
  add_body(self, u"00002000");
  self.head = u"00012000";
}

void character_game_instance::add_body(game_character &g,
                                       const std::u16string &val) {
  g.body = val;
  if (!avatar_data.contains(val)) {
    character_avatar_render &r = avatar_data[val];
    auto character_node = wz_resource::character;
    auto body_node = character_node->find(val + u".img");
    for (auto [k, v] : *body_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      r.data[k].resize(v[0]->children_count());
      for (uint8_t frame = 0; frame < v[0]->children_count(); frame++) {
        auto format2 = std::to_string(frame);
        auto body_frame_node = v[0]->get_child(format2);
        for (auto [key, val] : *body_frame_node->get_children()) {
          auto part_node = val[0];
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
            auto map_node = part_node->get_child(u"map");
            for (auto [mk, mv] : *map_node->get_children()) {
              if (bone_data.at(k).at(frame).bone_pos.contains(mk)) {
                auto p = bone_data.at(k).at(frame).bone_pos.at(mk);
                auto part_pos =
                    static_cast<wz::Property<wz::WzVec2D> *>(mv[0])->get();
                c.pos = {part_pos.x - p.x, part_pos.y - p.y};
                r.data[k][frame].push_back(c);
                break;
              }
            }
          }
        }
      }
    }
  }
}

fbs::CharacterT character_game_instance::load_fbs_character() {
  fbs::CharacterT c;

  c.appearance = std::make_unique<fbs::CharacterAppearanceT>();
  c.state = std::make_unique<fbs::LifeStateT>();

  if (self.accessory.has_value()) {
    c.appearance->accessory = self.accessory->id;
  }
  if (self.cap.has_value()) {
    c.appearance->cap = self.cap->id;
  }
  if (self.cape.has_value()) {
    c.appearance->cape = self.cape->id;
  }
  if (self.glove.has_value()) {
    c.appearance->glove = self.glove->id;
  }
  if (self.coat.has_value()) {
    c.appearance->coat = self.coat->id;
  }
  if (self.longcoat.has_value()) {
    c.appearance->longcoat = self.longcoat->id;
  }
  if (self.pant.has_value()) {
    c.appearance->pant = self.pant->id;
  }
  if (self.shield.has_value()) {
    c.appearance->shield = self.shield->id;
  }
  if (self.weapon.has_value()) {
    c.appearance->weapon = self.weapon->id;
  }
  if (self.shoes.has_value()) {
    c.appearance->shoes = self.shoes->id;
  }
  c.appearance->head =
      std::stoi(std::string{self.head.begin(), self.head.end()});
  c.appearance->body =
      std::stoi(std::string{self.body.begin(), self.body.end()});

  c.state->action = self.action_type;
  c.state->x = self.pos.x;
  c.state->y = self.pos.y;

  return c;
}