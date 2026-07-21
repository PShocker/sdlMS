#pragma once

#include <cstdint>
#include <functional>
#include <string>
class game_skill {
public:
  std::u16string id;
  std::function<void(int)> use;
  std::function<void()> frame;
  std::function<void()> passive;
  std::function<void()> end;

  uint64_t destory;
  uint64_t cd;
  uint8_t lv;
};