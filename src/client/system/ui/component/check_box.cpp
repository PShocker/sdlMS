#include "src/client/system/ui/component/check_box.h"

#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

namespace sdlms {

void CheckBox::Render() {
  SDL_Texture* texture = WzResource::LoadTexture(WzResource::ui_->find(
      checked ? u"Basic.img/CheckBox/0/1" : u"Basic.img/CheckBox/0/0"));

  // 有的 WZ 控件矩形比 checkbox 素材高（social 的 12x22、controller 的
  // 12x29），checkbox 应该在该矩形里居中，而不是贴着左上角画。
  // 调用方没给宽高时（quest 的调用），退回素材尺寸。
  if (rect.w <= 0.0f || rect.h <= 0.0f) {
    rect.w = static_cast<float>(texture->w);
    rect.h = static_cast<float>(texture->h);
  }
  const int offset_x =
      (static_cast<int>(rect.w) - texture->w) / 2;
  const int offset_y =
      (static_cast<int>(rect.h) - texture->h) / 2;

  SDL_FRect pos_rect{
      static_cast<float>(static_cast<int>(rect.x) + offset_x),
      static_cast<float>(static_cast<int>(rect.y) + offset_y),
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

bool CheckBox::Hover(SDL_FPoint mouse) {
  return SDL_PointInRectFloat(&mouse, &rect);
}

}  // namespace sdlms
