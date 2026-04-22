#pragma once

#include <cstdint>
#include <string>
class cursor_game_instance {
public:
  static inline std::u16string cursor_type = u"0";
  static inline uint8_t cursor_index;
  static inline uint32_t cursor_time;
};