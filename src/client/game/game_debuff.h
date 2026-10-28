#pragma once

#include <cstdint>
#include <string>

struct GameDebuff{
  enum class DebuffType {
    kDizz,
    kSlow,
  };
  DebuffType type;
  int64_t val;
  uint64_t delay = 0;
  uint64_t destroy = 0;
};