#pragma once

#include <cstdint>
#include <string>
struct GameItemBuff{
  std::u16string id;
  uint64_t destroy = 0;
  uint64_t duration;
};