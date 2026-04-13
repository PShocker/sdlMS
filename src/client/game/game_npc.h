#pragma once

#include <cstdint>
#include <string>
class game_npc {
public:
  uint32_t id;
  
  std::u16string action;

  uint32_t ani_index = 0;
  uint32_t ani_time;
};