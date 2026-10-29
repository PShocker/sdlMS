#include "src/client/system/ui/component/button.h"

#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <string>

namespace sdlms {

void Button::Move(float dx, float dy) {
  rect.x += dx;
  rect.y += dy;
}

bool Button::Hover() {
  mouse_over = false;
  if (enable && focused) {
    SDL_FRect hit = rect;
    mouse_over = SDL_PointInRectFloat(&Window::mouse_pos_, &hit);
  }
  return mouse_over;
}

void Button::Render() {
  if (node == nullptr) {
    return;
  }
  std::u16string state = u"normal";
  if (!enable) {
    state = u"disabled";
  } else if (Hover()) {
    state =
        (Window::mouse_state_ & SDL_BUTTON_LMASK) ? u"pressed" : u"mouseOver";
  }
  SDL_Texture* texture = WzResource::LoadTexture(node->find(state + u"/0"));
  if (alpha != 255) {
    SDL_SetTextureAlphaMod(texture, alpha);
  }
  SDL_FRect dst = rect;
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
}


}  // namespace sdlms
