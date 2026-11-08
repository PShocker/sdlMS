#pragma once

#include <cstdint>
#include <string>

namespace sdlms {

struct GameDebuff {
  enum class DebuffType {
    kDizz,
    kSlow,
    kDisorder,
    kThreaten,
    kBurn,
    kPoison,
  };
  DebuffType type;
  int64_t val;
  int64_t val2 = 0;
  uint64_t delay = 0;
  uint64_t destroy = 0;
};

}  // namespace sdlms
