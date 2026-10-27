#include "slider_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <cmath>
#include <optional>

void slider_ui_system::render_hslider(int x, int y, uint32_t val,
                                      uint32_t count, int32_t length, bool top,
                                      bool disable) {
  static auto normal = wz_resource::load_texture(
      wz_resource::ui->find(u"Slider.img/0/thumbNormal"));
  static auto pressed = wz_resource::load_texture(
      wz_resource::ui->find(u"Slider.img/0/thumbPressed"));
  static auto disabled = wz_resource::load_texture(
      wz_resource::ui->find(u"Slider.img/0/thumbDisabled"));
  static auto mouse_over = wz_resource::load_texture(
      wz_resource::ui->find(u"Slider.img/0/thumbMouseOver"));
  //   render
  SDL_FRect pos_rect;
  float persent = (float)val / count;
  pos_rect.x = x + persent * length;
  pos_rect.y = y;
  pos_rect.w = normal->w;
  pos_rect.h = normal->h;
  SDL_Texture *t;
  if (disable) {
    t = disabled;
  } else {
    auto mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        t = pressed;
      } else {
        t = mouse_over;
      }
    } else {
      t = normal;
    }
  }
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

std::optional<int> slider_ui_system::click_thumb(int x, int y, uint32_t val,
                                                 uint32_t count, int32_t length,
                                                 bool top, SDL_FPoint mouse) {
  SDL_FRect pos_rect = {
      static_cast<float>(x),
      static_cast<float>(y),
      static_cast<float>(length),
      static_cast<float>(14),
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