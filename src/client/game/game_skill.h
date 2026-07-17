#pragma once

#include <functional>
#include <string>
class game_skill {
public:
  std::u16string id;
  std::function<void()> use;
  std::function<void()> hit;
  std::function<void()> frame;
};