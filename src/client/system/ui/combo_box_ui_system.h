#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/window/window.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct combo_box {
  bool open;
  uint32_t w;
  std::u16string selected;
  std::vector<std::u16string> selects;
  uint8_t val;
};

class combo_box_ui_system {
public:
  static void render(float x, float y, combo_box cm, bool top, bool disable);
  static bool click(float x, float y, combo_box cm, bool top, bool disable);
};