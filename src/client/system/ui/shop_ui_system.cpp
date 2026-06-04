#include "shop_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "src/client/system/system.h"

void shop_ui_system::render_backgrnd() {
  static auto texture =
      wz_resource::load_texture(wz_resource::ui->find(u"Revive.img/back0"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void shop_ui_system::render_button() {
}

bool shop_ui_system::render() { return true; }