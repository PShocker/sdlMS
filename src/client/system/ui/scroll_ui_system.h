#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/window/window.h"
#include <cstdint>
class scroll_ui_system {
public:
  static void render_vscroll(float x, float y, uint32_t val, uint32_t count,
                             int32_t length, bool top, uint32_t size);
  static uint32_t click_vscroll(float x, float y, uint32_t val, uint32_t count,
                                int32_t length, bool top,
                                SDL_FPoint mouse = window::mouse_pos);
  static bool click_thumb(float x, float y, int32_t length,
                          SDL_FPoint mouse = window::mouse_pos);
};