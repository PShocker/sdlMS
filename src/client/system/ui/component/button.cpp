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
  // 贴图是缓存共享的：每次绘制都必须显式设置 alpha，并在结束后恢复。
  // 否则窗口淡入时某个 state 会被永久设成低 alpha，之后 hover 看起来发黑/发暗。
  SDL_SetTextureAlphaMod(texture, alpha);
  SDL_FRect dst = rect;
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
  SDL_SetTextureAlphaMod(texture, 255);
}


}  // namespace sdlms
