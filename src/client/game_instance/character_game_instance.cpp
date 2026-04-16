#include "character_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <format>
#include <string>

void character_game_instance::init_character_bone() {
  auto character_node = wz_resource::character;

  auto body_node = character_node->find(u"00002000.img");
  auto head_node = character_node->find(u"00012000.img");

  std::flat_map<std::u16string, wz::Node *> tmp_bone_map;
  for (auto [key, val] : *body_node->get_children()) {
    if (key == u"info") {
      continue;
    }
    auto action_node = val[0];
    for (uint8_t frame = 0; frame < action_node->children_count(); frame++) {
      auto format2 = std::to_string(frame);
      auto frame_node = action_node->get_child(format2);
      if (frame_node->get_child(u"action") == nullptr) {
        //  基础动作，身体
        for (auto [part_name, parts] : *frame_node->get_children()) {
          if (part_name == u"delay" || part_name == u"face") {
            continue;
          }
          auto part_node = parts[0];
          auto z = static_cast<wz::Property<std::u16string> *>(
                       part_node->get_child(u"z"))
                       ->get();
          tmp_bone_map.insert({z, part_node->get_child(u"map")});
        }
        // 头部
        auto format3 = std::u16string{format2.begin(), format2.end()};
        frame_node = head_node->find(key + u"/" + format3);
        for (auto [part_name, parts] : *frame_node->get_children()) {
          auto part_node = parts[0];
          if (part_node->type == wz::Type::UOL) {
            part_node =
                static_cast<wz::Property<wz::WzUOL> *>(part_node)->get_uol();
          }
          auto z = static_cast<wz::Property<std::u16string> *>(
                       part_node->get_child(u"z"))
                       ->get();
          tmp_bone_map.insert({z, part_node->get_child(u"map")});
        }
      } else {
        // 引用动作
        auto action = static_cast<wz::Property<std::u16string> *>(
                          frame_node->get_child(u"action"))
                          ->get();
        auto frame =
            static_cast<wz::Property<int> *>(frame_node->get_child(u"frame"))
                ->get();
        auto delay =
            static_cast<wz::Property<int> *>(frame_node->get_child(u"delay"))
                ->get();
        SDL_FPoint move = {0, 0};
        if (frame_node->get_child(u"move")) {
          auto v = static_cast<wz::Property<wz::WzVec2D> *>(
                       frame_node->get_child(u"move"))
                       ->get();
          move = {static_cast<float>(v.x), static_cast<float>(v.y)};
        }
        character_extern_action e_action = {.action = action,
                                            .frame =
                                                static_cast<uint8_t>(frame),
                                            .delay = delay,
                                            .move = move};
        ex_action.insert({key, e_action});
      }
    }
  }
  // 构建骨骼
  for (const auto &v : zmap) {
    if (tmp_bone_map.contains(v)) {
      auto map_node = tmp_bone_map.at(v);
      for (auto [key, val] : *map_node->get_children()) {
        auto v = static_cast<wz::Property<wz::WzVec2D> *>(val[0])->get();
        if (bone_data.contains(key)) {

        } else {
          bone_data[key] =
              SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};
        }
      }
    }
  }
}