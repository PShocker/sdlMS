#include "src/client/system/ui/component/slider.h"

#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <cmath>
#include <optional>

namespace sdlms {

void Slider::Render(float x, float y) const {
  static auto normal = WzResource::LoadTexture(
      WzResource::ui_->find(u"Slider.img/0/thumbNormal"));
  static auto pressed = WzResource::LoadTexture(
      WzResource::ui_->find(u"Slider.img/0/thumbPressed"));
  static auto disabled = WzResource::LoadTexture(
      WzResource::ui_->find(u"Slider.img/0/thumbDisabled"));
  static auto mouse_over = WzResource::LoadTexture(
      WzResource::ui_->find(u"Slider.img/0/thumbMouseOver"));
  //   render
  SDL_FRect pos_rect;
  float persent = (float)val / count;
  pos_rect.x = x + persent * length;
  pos_rect.y = y;
  pos_rect.w = normal->w;
  pos_rect.h = normal->h;
  SDL_Texture* t;
  if (disable) {
    t = disabled;
  } else {
    auto mouse_pos = Window::mouse_pos_;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        t = pressed;
      } else {
        t = mouse_over;
      }
    } else {
      t = normal;
    }
  }
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
}

std::optional<int> Slider::Click(float x, float y, SDL_FPoint mouse) const {
  SDL_FRect pos_rect = {
      x,
      y,
      static_cast<float>(length),
      14,
  };
  if (SDL_PointInRectFloat(&mouse, &pos_rect)) {
    auto dx = mouse.x - pos_rect.x;
    float percent = (float)dx / pos_rect.w;
    if (percent <= 0.02) {
      percent = 0;
    } else if (percent >= 0.98) {
      percent = 1;
    }
    auto val = std::round(percent * count);
    return val;
  }
  return std::nullopt;
}

}  // namespace sdlms
