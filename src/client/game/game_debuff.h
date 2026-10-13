#pragma once

#include <cstdint>
#include <string>

class game_debuff {
public:
  enum debuff_type {
    dizz,
    slow,
  };
  debuff_type type;
  int64_t val;
  uint64_t delay = 0;
  uint64_t destroy = 0;
};