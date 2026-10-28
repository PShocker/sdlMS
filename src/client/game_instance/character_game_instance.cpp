#include "src/client/game_instance/character_game_instance.h"

#include "src/client/game/game_character.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>
#include <wz/Wz.h>

#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {

void CharacterGameInstance::InitCharacterBone() {
  auto character_node = WzResource::character_;

  auto body_node = character_node->find(u"00002000.img");
  auto head_node = character_node->find(u"00012000.img");

  struct TmpBone {
    std::u16string name;
    wz::Node* node;
  };
  struct TmpBoneData {
    // z node
    std::flat_multimap<std::u16string, TmpBone> bones;
  };
  // action index
  std::flat_map<std::u16string, std::vector<TmpBoneData>> tmp_bone_map;
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
        bone_data_[key].resize(action_node->children_count());

        if (body_frame_node->get_child(u"delay")) {
          auto delay = static_cast<wz::Property<int>*>(
                           body_frame_node->get_child(u"delay"))
                           ->get();
          bone_data_[key][frame].delay = delay;
        }

        auto face = static_cast<wz::Property<int16_t>*>(
                        body_frame_node->get_child(u"face"))
                        ->get();
        bone_data_[key][frame].face = face;

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
                  static_cast<wz::Property<wz::WzUOL>*>(part_node)->get_uol();
            }
            if (!part_node->get_child(u"z")) {
              continue;
            }
            auto z = static_cast<wz::Property<std::u16string>*>(
                         part_node->get_child(u"z"))
                         ->get();
            auto map_node = part_node->get_child(u"map");
            tmp_bone_map[key][frame].bones.insert({z, {k, part_node}});
          }
        }
      } else {
        CharacterExternAction e_action;
        // 引用动作
        e_action.action = static_cast<wz::Property<std::u16string>*>(
                              body_frame_node->get_child(u"action"))
                              ->get();
        e_action.frame = static_cast<wz::Property<int>*>(
                             body_frame_node->get_child(u"frame"))
                             ->get();
        e_action.delay = static_cast<wz::Property<int>*>(
                             body_frame_node->get_child(u"delay"))
                             ->get();
        SDL_FPoint Move = {0, 0};
        if (body_frame_node->get_child(u"move")) {
          auto v = static_cast<wz::Property<wz::WzVec2D>*>(
                       body_frame_node->get_child(u"move"))
                       ->get();
          e_action.move = {static_cast<float>(v.x), static_cast<float>(v.y)};
        }
        extern_action_[key].push_back(e_action);
      }
    }
  }
  for (const auto [k, v] : tmp_bone_map) {
    for (uint8_t i = 0; i < v.size(); i++) {
      const auto& tbd = v[i];
      for (const auto& zp : zmap) {
        if (!tbd.bones.contains(zp)) {
          continue;
        }
        // const auto &bones = tbd.bones.at(zp);
        auto [beg, End] = tbd.bones.equal_range(zp);
        for (auto it = beg; it != End; ++it) {
          auto& bones = it->second;
          // 构建骨骼
          std::u16string parent_bone_name = u"";
          for (auto [pk, pv] : *bones.node->get_child(u"map")->get_children()) {
            if (bone_data_[k][i].bone_pos.contains(pk)) {
              parent_bone_name = pk;
            }
          }
          if (parent_bone_name == u"") {
            // 无父节点
            for (auto [pk, pv] :
                 *bones.node->get_child(u"map")->get_children()) {
              auto vec = static_cast<wz::Property<wz::WzVec2D>*>(pv[0])->get();
              bone_data_[k][i].bone_pos[pk] = {float(vec.x), float(vec.y)};
            }
            bone_data_[k][i].part_pos[bones.name] = {0, 0};
          } else {
            // 有父节点
            auto parent_bone_pos =
                bone_data_[k][i].bone_pos.at(parent_bone_name);
            auto parent_vec = bones.node->find(u"map/" + parent_bone_name);
            auto vec =
                static_cast<wz::Property<wz::WzVec2D>*>(parent_vec)->get();
            SDL_FPoint cur_pos{parent_bone_pos.x - vec.x,
                               parent_bone_pos.y - vec.y};
            bone_data_[k][i].part_pos[bones.name] = cur_pos;
            for (auto [pk, pv] :
                 *bones.node->get_child(u"map")->get_children()) {
              if (bone_data_[k][i].bone_pos.contains(pk)) {
                continue;
              }
              auto vec = static_cast<wz::Property<wz::WzVec2D>*>(pv[0])->get();
              bone_data_[k][i].bone_pos[pk] = {float(cur_pos.x + vec.x),
                                               float(cur_pos.y + vec.y)};
            }
          }
        }
      }
    }
  }
}

void CharacterGameInstance::InitDefaultClothes() {
  GameCharacter g;
  AddCoat(g, u"01040000");
  AddPants(g, u"01060000");
  return;
}

void CharacterGameInstance::LoadSelfCharacter() {
  AddBody(self_, u"00002000");
  AddHead(self_, u"00012000");
  AddCoat(self_, u"01040002");
  AddCap(self_, u"01002005");
  AddWeapon(self_, u"01472002");
  AddPants(self_, u"01060001");
  AddFace(self_, u"00020000");
  AddHair(self_, u"00030000");
  AddShield(self_, u"01092003");
  AddEar(self_, u"humanEar");

  AddCapDeco(self_, u"01007002");
  AddRing0Deco(self_, u"01112219");
  AddRing1Deco(self_, u"01112108");

  self_.weapon->scroll = {
      {u"02040000", true},
      {u"02040001", true},
      {u"02040002", true},
      {u"02040003", true},
  };
}

std::flat_set<std::u16string> CharacterGameInstance::SplitVslot(
    const std::u16string& vslot) {
  std::flat_set<std::u16string> result;
  for (size_t i = 0; i < vslot.size(); i += 2) {
    result.insert(vslot.substr(i, 2));
  }
  return result;
}

std::u16string CharacterGameInstance::SplitIslot(const std::u16string& islot) {
  auto result = SplitVslot(islot);
  for (auto k : zmap2) {
    if (result.contains(k)) {
      return k;
    }
  }
  return islot;
}

void CharacterGameInstance::AddBody(GameCharacter& g,
                                    const std::u16string& val) {
  g.body = val;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddHead(GameCharacter& g,
                                    const std::u16string& val) {
  g.head = val;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddEar(GameCharacter& g,
                                   const std::u16string& val) {
  g.ear = val;
  std::flat_set<std::u16string> hide_ear = {
      u"humanEar",
      u"ear",
      u"lefEar",
      u"highlefEar",
  };
  // 从 hide_part 中移除所有耳朵
  for (const auto& ear : hide_ear) {
    g.hide_part.erase(ear);
  }
  hide_ear.erase(val);
  g.hide_part.insert_range(hide_ear);
}

void CharacterGameInstance::AddCoat(GameCharacter& g,
                                    const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.coat = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddCoatDeco(GameCharacter& g,
                                        const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.coat_deco = g_deco;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddCap(GameCharacter& g,
                                   const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.cap = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddCapDeco(GameCharacter& g,
                                       const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.cap_deco = g_deco;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddPants(GameCharacter& g,
                                     const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.pant = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddPantsDeco(GameCharacter& g,
                                         const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.pant_deco = g_deco;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddWeapon(GameCharacter& g,
                                      const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.weapon = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddWeaponDeco(GameCharacter& g,
                                          const std::u16string& val) {
  if (!g.weapon.has_value()) {
    return;
  }
  auto WeaponEnum = g.weapon->id;
  std::u16string sub = WeaponEnum.substr(2, 2);
  GameDecoItem g_deco;
  g_deco.id = val;
  g.weapon_deco = g_deco;
  AddAvatarCache(val, sub);
}

void CharacterGameInstance::AddShield(GameCharacter& g,
                                      const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.shield = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddShieldDeco(GameCharacter& g,
                                          const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.shield_deco = g_deco;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddCape(GameCharacter& g,
                                    const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.cape = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddCapeDeco(GameCharacter& g,
                                        const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.cape_deco = g_deco;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddAccessory(GameCharacter& g,
                                         const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.accessory = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddAccessoryDeco(GameCharacter& g,
                                             const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.accessory_deco = g_deco;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddGlove(GameCharacter& g,
                                     const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.glove = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddGloveDeco(GameCharacter& g,
                                         const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.glove_deco = g_deco;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddLongcoat(GameCharacter& g,
                                        const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.longcoat = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddLongcoatDeco(GameCharacter& g,
                                            const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.longcoat_deco = g_deco;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddShoes(GameCharacter& g,
                                     const std::u16string& val) {
  GameEquipItem g_equip;
  g_equip.id = val;
  g.shoes = g_equip;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddShoesDeco(GameCharacter& g,
                                         const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.shoes_deco = g_deco;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddHair(GameCharacter& g,
                                    const std::u16string& val) {
  g.hair = val;
  AddAvatarCache(val, u"");
}

void CharacterGameInstance::AddFace(GameCharacter& g,
                                    const std::u16string& val) {
  GameFace g_face;
  g_face.id = val;
  g.face = g_face;
  if (!face_data_.contains(val)) {
    auto character_node = WzResource::character_;
    auto face_node = character_node->find(u"Face/" + val + u".img");
    for (auto [k, v] : *face_node->get_children()) {
      if (k == u"info") {
        continue;
      }
      auto& f = face_data_[val];
      for (const auto& [bk, bv] : bone_data_) {
        for (const auto& cbd : bv) {
          if (!cbd.bone_pos.contains(u"brow")) {
            continue;
          }
          auto brow_pos = cbd.bone_pos.at(u"brow");
          std::vector<CharacterAvatar> vc;
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
            CharacterAvatar c;
            auto f_brow = static_cast<wz::Property<wz::WzVec2D>*>(
                              f_node->find(u"map/brow"))
                              ->get();
            c.texture = WzResource::LoadTexture(f_node);
            // 直接写face
            c.z = u"face";
            auto ori =
                static_cast<wz::Property<wz::WzVec2D>*>(f_node->find(u"origin"))
                    ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            c.pos = {brow_pos.x - f_brow.x, brow_pos.y - f_brow.y};
            vc.push_back(c);

            // delay
            uint32_t delay = 0;
            if (f_node->find(u"../delay")) {
              delay = static_cast<wz::Property<int>*>(f_node->find(u"../delay"))
                          ->get();
            }
            vd.push_back(delay);
          }
          f.delay[k] = vd;
          f.data[k].data[bk].push_back(vc);
          f.data[k].islot = u"Fc";
          f.data[k].vslot = {u"Fc"};  // face只占用Fc
        }
      }
    }
  }
}

void CharacterGameInstance::AddRing0Deco(GameCharacter& g,
                                         const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.ring0_deco = g_deco;
}

void CharacterGameInstance::AddRing1Deco(GameCharacter& g,
                                         const std::u16string& val) {
  GameDecoItem g_deco;
  g_deco.id = val;
  g.ring1_deco = g_deco;
}

void CharacterGameInstance::AddAvatarCache(const std::u16string& val,
                                           const std::u16string& sub) {
  std::u16string v = val;
  if (!sub.empty()) {
    v = val + u"/" + sub;
  }
  if (!avatar_data_.contains(v)) {
    CharacterAvatarRender& r = avatar_data_[v];
    auto character_node = WzResource::character_;
    auto type = EquipGameInstance::LoadEquipType(val);
    wz::Node* node;
    if (type == u"Body" || type == u"Head") {
      node = character_node->find(val + u".img");
    } else {
      node = character_node->find(type + u"/" + val + u".img");
    }
    r.islot =
        static_cast<wz::Property<std::u16string>*>(node->find(u"info/islot"))
            ->get();
    r.islot = SplitIslot(r.islot);
    r.vslot = SplitVslot(
        static_cast<wz::Property<std::u16string>*>(node->find(u"info/vslot"))
            ->get());
    if (!sub.empty()) {
      node = node->get_child(sub);
    }
    auto child = *node->get_children();
    child.erase(u"info");
    if (type == u"Hair" || type == u"Cap" || type == u"Head") {
      child[u"dead"] = {child[u"jump"].at(0)};
    }
    for (auto [k, v] : child) {
      if (!bone_data_.contains(k)) {
        continue;
      }
      auto size = bone_data_[k].size();
      r.data[k].resize(size);
      for (uint8_t frame = 0; frame < size; frame++) {
        auto format2 = std::to_string(frame);
        auto frame_node = v[0]->get_child(format2);
        if (frame_node == nullptr) {
          continue;
        }
        for (auto [bk, bv] : *frame_node->get_children()) {
          auto part_node = bv[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL>*>(part_node)->get_uol();
          }
          if (part_node->type == wz::Type::Canvas) {
            CharacterAvatar c;
            c.texture = WzResource::LoadTexture(part_node);
            c.z = static_cast<wz::Property<std::u16string>*>(
                      part_node->get_child(u"z"))
                      ->get();
            auto ori = static_cast<wz::Property<wz::WzVec2D>*>(
                           part_node->get_child(u"origin"))
                           ->get();
            c.origin = {static_cast<float>(ori.x), static_cast<float>(ori.y)};
            auto map_node =
                part_node->get_child(u"map")->get_children()->begin();
            auto part_name = map_node->first;
            auto part_val = map_node->second[0];
            auto part_val_pos =
                static_cast<wz::Property<wz::WzVec2D>*>(part_val)->get();
            auto parent_pos = bone_data_[k][frame].bone_pos.at(part_name);

            c.pos = {parent_pos.x - part_val_pos.x,
                     parent_pos.y - part_val_pos.y};
            c.name = bk;
            r.data[k][frame].push_back(c);
          }
        }
      }
    }
  }
}

}  // namespace sdlms
