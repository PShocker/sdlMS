#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/window/window.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct ComboBox {
  bool open;
  uint32_t w;
  std::u16string selected;
  std::vector<std::u16string> selects;
  uint8_t val;
};

class ComboBoxUiSystem {
public:
  static void render(int x, int y, ComboBox cm, bool top, bool disable);
  static bool click(int x, int y, ComboBox cm, bool top, bool disable);
};