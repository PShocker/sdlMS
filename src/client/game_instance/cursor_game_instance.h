#pragma once

#include "src/client/game/game_item.h"
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
  static inline bool (*modal_overlay)(void);

  // 点击装备,背包,技能,键盘按钮
  enum cursor_hand_type {
    equipment,
    package,
    skill,
    keybind,
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

  static void server_cursor_drop(const DropT &dt);
};