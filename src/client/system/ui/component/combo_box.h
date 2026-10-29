#pragma once

#include "src/client/window/window.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

// A drop-down selector backed by Basic.img/ComboBox.
//
// `w` is the pixel width of the closed box, `selected` the current entry and
// `selects` the entries shown while `open`. `val` is the selected index.
struct ComboBox {
  bool open;
  uint32_t w;
  std::u16string selected;
  std::vector<std::u16string> selects;
  uint8_t val;
  SDL_FRect rect = {};   // top-left position, set by the owning UI
  bool disable = false;  // draws the disabled box, never hovers

  // Draws the box at `rect`, together with its drop-down list when `open`.
  void Render() const;

  // Returns true when the click at `mouse` was handled.
  bool Click(SDL_FPoint mouse = Window::mouse_pos_) const;
};

}  // namespace sdlms
