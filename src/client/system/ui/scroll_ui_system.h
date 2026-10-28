#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_SCROLL_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_SCROLL_UI_SYSTEM_H_

#include "src/client/window/window.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>

namespace sdlms {

class ScrollUiSystem {
 public:
  static void RenderVscroll(float x, float y, uint32_t val, uint32_t count,
                            int32_t length, bool top, uint32_t size);
  static uint32_t ClickVscroll(float x, float y, uint32_t val, uint32_t count,
                               int32_t length, bool top,
                               SDL_FPoint mouse = Window::mouse_pos_);
  static bool ClickThumb(float x, float y, int32_t length,
                         SDL_FPoint mouse = Window::mouse_pos_);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_SCROLL_UI_SYSTEM_H_
