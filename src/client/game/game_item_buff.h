#pragma once

#include <cstdint>
#include <string>
class game_item_buff {
public:
  std::u16string id;
  uint64_t destroy = 0;
  uint64_t duration;
};