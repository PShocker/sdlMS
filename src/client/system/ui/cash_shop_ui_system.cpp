#include "cash_shop_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

void cash_shop_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/customize/canvas:top"));
  SDL_FRect pos_rect = {
      pos_rect.x + 160,
      pos_rect.y + 85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
}

bool cash_shop_ui_system::render() {
  render_backgrnd();
  return false;
}