#pragma once

#include "src/client/window/window.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>

namespace sdlms {

// A horizontal slider backed by Slider.img.
//
// The thumb sits at `val / count` of a track that is `length` pixels wide; the
// owning UI places the track and keeps `val` in sync.
struct Slider {
  uint32_t val = 0;      // current value
  uint32_t count = 100;  // value at the right edge of the track
  int32_t length = 0;    // track width in pixels
  bool disable = false;  // draws the disabled thumb, never hovers
  SDL_FPoint pos = {};   // top-left of the track, set by the owning UI

  Slider(uint32_t val, uint32_t count, int32_t length, bool disable = false)
      : val(val), count(count), length(length), disable(disable) {}

  // Draws the thumb at `pos` for the current value.
  void Render() const;

  // Returns the value under `mouse` when it hits the track, otherwise
  // `std::nullopt`.
  std::optional<int> Click(SDL_FPoint mouse = Window::mouse_pos_) const;
};

}  // namespace sdlms
