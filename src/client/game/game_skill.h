#pragma once

#include <cstdint>
#include <functional>
#include <string>
class game_skill {
public:
  std::u16string id;
  std::function<void()> use;
  std::function<void()> used;
  std::function<bool()> frame;
  std::function<void()> passive;

  uint8_t lv;
};