#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
#include <vector>

// 定义枚举类
enum class input_type {
  INPUT_ACTION,
  INPUT_UI,
  INPUT_ITEM,
  INPUT_SKILL,
};

enum class action_type : uint32_t {
  ACTION_UP,
  ACTION_DOWN,
  ACTION_LEFT,
  ACTION_RIGHT,
  ACTION_JUMP,
  ACTION_ATTACK,
};

enum class ui_type : uint32_t {
  UI_PACKAGE,
};

class game_input {
public:
  input_type type;
  uint32_t val;
};