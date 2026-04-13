#pragma once

#include <cstdint>
#include <string>
class game_obj {
public:
  std::u16string path;

  uint32_t ani_index = 0;
  uint32_t ani_time;
};