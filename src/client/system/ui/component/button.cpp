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
  auto* texture_node = node->find(state + u"/0");
  bool disabled_fallback = false;
  if (texture_node == nullptr && state == u"disabled") {
    // 部分按钮没有 disabled/0，用 normal/0 灰度显示。
    texture_node = node->find(u"normal/0");
    disabled_fallback = true;
  }
  if (texture_node == nullptr) {
    return;
  }
  SDL_Texture* texture = WzResource::LoadTexture(texture_node);
  if (texture == nullptr) {
    return;
  }
  if (disabled_fallback) {
    SDL_SetTextureColorMod(texture, 128, 128, 128);
  }
  SDL_SetTextureAlphaMod(texture, alpha);
  SDL_FRect dst = rect;
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
  SDL_SetTextureColorMod(texture, 255, 255, 255);
  SDL_SetTextureAlphaMod(texture, 255);}


}  // namespace sdlms
