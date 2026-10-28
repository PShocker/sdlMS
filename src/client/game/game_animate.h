#pragma once

#include <cstdint>
#include <vector>
class GameAnimate {
public:
  uint8_t ani_index = 0;
  uint32_t ani_time = 0;
  std::vector<uint32_t> ani_delay;
};