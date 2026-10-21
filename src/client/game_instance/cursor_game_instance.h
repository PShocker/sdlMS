#pragma once

#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <optional>
#include <string>

using namespace fbs;

class cursor_game_instance {
public:
  static inline std::u16string cursor_type = u"0";
  static inline uint8_t cursor_index;
  static inline uint32_t cursor_time;

  static inline bool (*cursor_ui)(void);
  static inline bool cursor_vscr;
  static inline bool (*modal_overlay)(void);

  // 点击装备,背包,技能,键盘按钮
  enum cursor_hand_type {
    equipment,
    deco,
    package,
    skill,
    keybind,
  };
  enum cursor_keybind_type { // sub_val
    keybind_ui_menu,
    keybind_face_hit,
    keybind_face_smile,
    keybind_face_troubled,
    keybind_face_cry,
    keybind_face_angry,
    keybind_face_bewildered,
    keybind_face_stunned,
    keybind_ui_toAll,
    keybind_ui_toParty,
    keybind_ui_quest,
    keybind_ui_worldMap,
    keybind_ui_equip,
    keybind_ui_package,
    keybind_ui_buddy,
    keybind_ui_party,
    keybind_ui_mapleChat,
    keybind_ui_quickSlot,
    keybind_ui_keyBind,
    keybind_ui_characterStat,
    keybind_ui_whisper,
    keybind_ui_skill,
    keybind_ui_craft,
    keybind_ui_mailBox,
    keybind_ui_chatPlus,
    keybind_ui_miniMap,
    keybind_ui_changeChannel,
    keybind_ui_option,
    keybind_action_pick,
    keybind_action_sit,
    keybind_action_attack,
    keybind_action_jump,
    keybind_action_harvest,
    keybind_action_up,
    keybind_action_down,
    keybind_action_left,
    keybind_action_right,
    keybind_face_vomit,
  };
  struct cursor_hand_data {
    cursor_hand_type type;
    uint32_t val;
    uint32_t sub_val;
  };
  enum cursor_hand_net_type {
    drop,
  };
  struct cursor_hand_net_data {
    cursor_hand_net_type type;
    uint64_t id;
  };

  static inline std::optional<cursor_hand_net_data> cursor_hand_net;
  static inline std::optional<cursor_hand_data> cursor_hand;
};