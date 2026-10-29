#include "src/client/component/button.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <string>

namespace sdlms {

void Button::Move(float dx, float dy) {
  rect.x += dx;
  rect.y += dy;
}

bool Button::CursorIn() const {
  if (node == nullptr || !enable || !focused) {
    return false;
  }
  SDL_FRect hit = rect;
  return SDL_PointInRectFloat(&Window::mouse_pos_, &hit);
}

void Button::Render() const {
  if (node == nullptr) {
    return;
  }
  std::u16string state = u"normal";
  if (!enable) {
    state = u"disabled";
  } else if (CursorIn()) {
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

bool DispatchButtonClick(const std::vector<Button>& components,
                         bool play_sound) {
  for (const Button& button : components) {
    if (button.callback == nullptr || !button.enable || !button.focused) {
      continue;
    }
    SDL_FRect hit = button.rect;
    if (!SDL_PointInRectFloat(&Window::mouse_pos_, &hit)) {
      continue;
    }
    if (play_sound) {
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    }
    button.callback();
    return true;
  }
  return false;
}

}  // namespace sdlms
