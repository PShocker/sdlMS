#include "src/client/system/ui/cash_shop_ui_system.h"

#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>

namespace sdlms {

void CashShopUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"Login.img/NewChar/customize/canvas:top"));
  SDL_FRect pos_rect = {
      160,
      85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
}

bool CashShopUiSystem::Render() {
  RenderBackgrnd();
  return true;
}

bool CashShopUiSystem::Event(SDL_Event* Event) {
  bool r = true;
  switch (Event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (Event->button.button == SDL_BUTTON_LEFT) {
        r = false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (Event->button.button == SDL_BUTTON_LEFT) {
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

}  // namespace sdlms
