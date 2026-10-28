#pragma once

#include <cstdint>
#include <string>

class GameDebuff {
public:
  enum DebuffType {
    dizz,
    slow,
  };
  DebuffType type;
  int64_t val;
  uint64_t delay = 0;
  uint64_t destroy = 0;
};