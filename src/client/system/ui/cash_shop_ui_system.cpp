#include "cash_shop_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

void CashShopUiSystem::render_backgrnd() {
  static auto backgrnd = WzResource::load_texture(
      WzResource::ui->find(u"Login.img/NewChar/customize/canvas:top"));
  SDL_FRect pos_rect = {
      160,
      85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
}

bool CashShopUiSystem::render() {
  render_backgrnd();
  return true;
}

bool CashShopUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      r = false;
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}