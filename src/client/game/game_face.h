#pragma once

#include <cstdint>
#include <string>
class game_face {
public:
  std::u16string id;
  std::u16string type;
  uint32_t time;
  uint8_t index;
};