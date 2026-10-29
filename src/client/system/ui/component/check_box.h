#pragma once

#include "src/client/window/window.h"

#include <SDL3/SDL_rect.h>

namespace sdlms {

// A checkbox backed by Basic.img/CheckBox/0.
//
// `checked` selects the `0/1` image; `0/0` is the unchecked box. Render()
// also fills rect.w / rect.h from the texture, so owners can lay out labels
// relative to it afterwards.
struct CheckBox {
  SDL_FRect rect = {};  // absolute; w/h are set by Render()
  bool checked = false;

  // Draws the box in its checked or unchecked state.
  void Render();

  // True while the cursor is over the box.
  bool Hover(SDL_FPoint mouse = Window::mouse_pos_);
};

}  // namespace sdlms
