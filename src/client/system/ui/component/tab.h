#pragma once

#include "src/client/window/window.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

// A horizontal strip of tabs sharing one WZ node.
//
// `node` holds the two image groups named `on_state`/`off_state` (for example
// `enabled` and `disabled`, or `selected` and `normal`); `rects` gives each
// tab's absolute screen rect. The owning UI keeps `selected` in sync and
// reacts to Click().
struct Tab {
  wz::Node* node = nullptr;
  std::u16string on_state = u"enabled";    // child holding the selected image
  std::u16string off_state = u"disabled";  // child holding the other images
  std::vector<SDL_FRect> rects;            // absolute, one per tab
  uint8_t selected = 0;

  // Draws every tab at the top-left of its rect.
  void Render() const;

  // Returns the index of the tab under `mouse`, otherwise `std::nullopt`.
  std::optional<uint8_t> Click(SDL_FPoint mouse = Window::mouse_pos_) const;
};

}  // namespace sdlms
