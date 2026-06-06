#pragma once

#include <cstdint>
class scroll_ui_system {
public:
  static void render_vscroll(float x, float y, uint32_t val, uint32_t count,
                             int32_t length, bool top);
  static uint32_t click_vscroll(float x, float y, uint32_t val, uint32_t count,
                                int32_t length, bool top);
};