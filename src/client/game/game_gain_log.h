#pragma once

#include <cstdint>
#include <string>
class game_gain_log {
public:
  std::u16string id;
  uint64_t num;
  uint64_t destory;

  bool experience;
};