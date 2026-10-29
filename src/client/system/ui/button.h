#pragma once

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

namespace sdlms {

// A clickable UI button backed by a WZ image node.
//
// `node` is expected to carry the standard MapleStory button children:
// `normal/0`, `mouseOver/0`, `pressed/0` and `disabled/0`.
struct Button {
  wz::Node* node = nullptr;
  SDL_FRect rect = {};  // absolute, screen-space rectangle
  bool enable = true;   // disabled buttons draw `disabled/0`, never hover
  bool focused = true;  // false while another window covers the owning UI
  uint8_t alpha = 255;  // 0-255, used by fading windows

  // Offsets the button rectangle, e.g. by the owning window position.
  void Move(float dx, float dy);

  // True when the cursor is over an enabled, focused button.
  bool CursorIn() const;

  // Draws the button in its normal, hovered, pressed or disabled state.
  void Render() const;
};

}  // namespace sdlms
