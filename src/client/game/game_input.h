#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
#include <vector>

// // 定义枚举类
// enum class input_type {
//   Action,
//   Ui,
//   Item,
//   Skill,
// };

// enum class action_type : uint32_t {
//   Up,
//   Down,
//   Left,
//   Right,
//   Jump,
//   Attack,
// };

// enum class ui_type : uint32_t {
//   Package,
// };

class game_input {
public:
  std::string type;
  std::string val;
  std::string val2;
};