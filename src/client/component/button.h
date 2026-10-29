#pragma once

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <functional>
#include <vector>

namespace sdlms {

// A clickable UI component backed by a WZ image node.
//
// `node` is expected to carry the standard MapleStory button children:
// `normal/0`, `mouseOver/0`, `pressed/0` and `disabled/0`. A component without
// a node is still clickable; it is rendered by its owning UI.
struct Button {
  using Callback = std::function<void()>;

  wz::Node* node = nullptr;
  SDL_FRect rect = {};  // absolute, screen-space rectangle
  Callback callback;    // invoked when the button is clicked
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

// Fires the callback of the first enabled component under the cursor.
//
// Returns true when a click was consumed. `play_sound` plays the standard
// button click sound, mirroring the behaviour of each UI before the
// components were introduced.
bool DispatchButtonClick(const std::vector<Button>& components,
                         bool play_sound);

}  // namespace sdlms
