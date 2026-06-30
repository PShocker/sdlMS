#include "context_menu_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

void context_menu_ui_system::render_backgrnd() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/t"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

bool context_menu_ui_system::render() { return true; }