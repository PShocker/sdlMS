#pragma once

#include "src/client/window/window.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>

namespace sdlms {

// A vertical scrollbar matching Basic.img/VScr4.
//
// `val` is the current top row, `count` the total number of rows and `size`
// the number of rows visible at once; `length` is the pixel height of the
// track. The owning UI draws the scrollbar itself and keeps `val` in sync.
struct Scroll {
  uint32_t val = 0;    // first visible row
  uint32_t count = 0;  // total rows
  int32_t length = 0;  // track height in pixels
  uint32_t size = 0;   // rows visible at once

  Scroll(uint32_t val, uint32_t count, int32_t length, uint32_t size = 0)
      : val(val), count(count), length(length), size(size) {}

  // Draws the scrollbar at (x, y); `top` marks the owning UI as focused.
  void Render(float x, float y, bool top) const;

  // Returns the page the cursor clicks at (x, y), or `val` when it misses.
  uint32_t Click(float x, float y, bool top,
                 SDL_FPoint mouse = Window::mouse_pos_) const;

  // True when (x, y) points at the draggable thumb.
  bool ClickThumb(float x, float y,
                  SDL_FPoint mouse = Window::mouse_pos_) const;
};

}  // namespace sdlms
