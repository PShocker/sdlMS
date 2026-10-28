#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/window/window.h"
#include <cstdint>
#include <optional>
class SliderUiSystem {
public:
  static void render_hslider(int x, int y, uint32_t val, uint32_t count,
                             int32_t length, bool top, bool disable);
  static std::optional<int> click_thumb(int x, int y, uint32_t val,
                                        uint32_t count, int32_t length,
                                        bool top,
                                        SDL_FPoint mouse = Window::mouse_pos);
};