#include "src/client/system/ui/component/check_box.h"

#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

namespace sdlms {

void CheckBox::Render() {
  SDL_Texture* texture = WzResource::LoadTexture(WzResource::ui_->find(
      checked ? u"Basic.img/CheckBox/0/1" : u"Basic.img/CheckBox/0/0"));
  rect.w = static_cast<float>(texture->w);
  rect.h = static_cast<float>(texture->h);
  SDL_FRect pos_rect{rect.x, rect.y, rect.w, rect.h};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

bool CheckBox::Hover(SDL_FPoint mouse) {
  return SDL_PointInRectFloat(&mouse, &rect);
}

}  // namespace sdlms
