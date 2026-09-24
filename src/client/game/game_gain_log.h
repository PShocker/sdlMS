#pragma once

#include <cstdint>
#include <string>

enum class gain_enum { item, experience };

class game_gain_log {
public:
  std::u16string id;
  int32_t num;
  uint64_t destroy;
  uint8_t order;

  gain_enum type;
};