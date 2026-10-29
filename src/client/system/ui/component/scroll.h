#pragma once

#include "src/client/window/window.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>

namespace sdlms {

// A vertical scrollbar matching Basic.img/VScr4.
//
// `val` is the current top row, `count` the total number of rows and `size`
// the number of rows visible at once; `length` is the pixel height of the
// track. The owning UI places the track and keeps `val` in sync.
struct Scroll {
  uint32_t val = 0;     // first visible row
  uint32_t count = 0;   // total rows
  int32_t length = 0;   // track height in pixels
  uint32_t size = 0;    // rows visible at once
  SDL_FPoint pos = {};  // top-left of the track, set by the owning UI
  bool top = false;     // owning UI focused, set by the owning UI

  Scroll(uint32_t val, uint32_t count, int32_t length, uint32_t size = 0)
      : val(val), count(count), length(length), size(size) {}

  // Draws the scrollbar at `pos`.
  void Render() const;

  // Returns the page the cursor clicks at `mouse`, or `val` when it misses.
  uint32_t Click(SDL_FPoint mouse = Window::mouse_pos_) const;

  // True when `mouse` is inside the draggable track (not the prev/next buttons).
  bool InTrack(SDL_FPoint mouse = Window::mouse_pos_) const;

  // Drags the thumb to `mouse.y`: x is forced to the track and y is clamped,
  // then the page is calculated the same way as Click().
  uint32_t Drag(SDL_FPoint mouse = Window::mouse_pos_) const;
};

}  // namespace sdlms
