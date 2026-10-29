#include "src/client/system/ui/component/combo_box.h"

#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

namespace sdlms {

void ComboBox::Render(int x, int y, bool top, bool disable) const {
  SDL_Texture* l = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/ComboBox/normal/0"));
  SDL_Texture* c = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/ComboBox/normal/1"));
  SDL_Texture* r = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/ComboBox/normal/2"));
  if (disable) {
    l = WzResource::LoadTexture(
        WzResource::ui_->find(u"Basic.img/ComboBox/disabled/0"));
    c = WzResource::LoadTexture(
        WzResource::ui_->find(u"Basic.img/ComboBox/disabled/1"));
    r = WzResource::LoadTexture(
        WzResource::ui_->find(u"Basic.img/ComboBox/disabled/2"));
  } else {
    auto mouse_pos = Window::mouse_pos_;
    SDL_FRect pos_r;
    pos_r.x = x;
    pos_r.y = y;
    pos_r.w = w;
    pos_r.h = 17;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_r)) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        l = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/ComboBox/normal/0"));
        c = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/ComboBox/normal/1"));
        r = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/ComboBox/normal/2"));
      } else {
        l = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/ComboBox/normal/0"));
        c = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/ComboBox/normal/1"));
        r = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/ComboBox/mouseOver/2"));
      }
    }
  }
  SDL_FRect pos_rect;
  pos_rect.x = x;
  pos_rect.y = y;
  pos_rect.w = l->w;
  pos_rect.h = l->h;
  SDL_RenderTexture(Window::renderer_, l, nullptr, &pos_rect);
  pos_rect.x += l->w;
  pos_rect.w = w - l->w - r->w;
  SDL_RenderTextureTiled(Window::renderer_, c, nullptr, 1, &pos_rect);
  pos_rect.x += pos_rect.w;
  pos_rect.w = r->w;
  SDL_RenderTexture(Window::renderer_, r, nullptr, &pos_rect);
  Freetype::LoadSize(12);
  Freetype::LoadColor(0, 0, 0, 255);
  Freetype::DrawLine(selected, x + 10, pos_rect.y);
  if (open) {
    auto lh = Freetype::LoadLh();
    auto h = selects.size() * lh * 1.3;
    SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 255);
    SDL_FRect pos_rr;
    pos_rr.x = x;
    pos_rr.y = y + l->h;
    pos_rr.w = pos_rect.w;
    pos_rr.h = pos_rect.h;
    SDL_RenderRect(Window::renderer_, &pos_rr);
    SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 255);
    SDL_RenderFillRect(Window::renderer_, &pos_rr);
    auto base_y = pos_rr.y;
    for (int i = 0; i < selects.size(); i++) {
      auto text = selects[i];
      Freetype::DrawLine(text, x + 10, base_y);
      base_y += lh * 1.3;
    }
  }
}

bool ComboBox::Click(int x, int y, bool top, bool disable) const {
  return false;
}

}  // namespace sdlms
