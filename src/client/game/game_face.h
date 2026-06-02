#pragma once

#include <cstdint>
#include <string>
class game_face {
public:
  std::u16string id;
  std::u16string action = u"default";
  uint32_t time = 0;
  uint8_t index = 0;
};