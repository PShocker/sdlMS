#include "combo_box_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

void ComboBoxUiSystem::render(int x, int y, ComboBox cm, bool top,
                                 bool disable) {
  SDL_Texture *l = WzResource::load_texture(
      WzResource::ui->find(u"Basic.img/ComboBox/normal/0"));
  SDL_Texture *c = WzResource::load_texture(
      WzResource::ui->find(u"Basic.img/ComboBox/normal/1"));
  SDL_Texture *r = WzResource::load_texture(
      WzResource::ui->find(u"Basic.img/ComboBox/normal/2"));
  if (disable) {
    l = WzResource::load_texture(
        WzResource::ui->find(u"Basic.img/ComboBox/disabled/0"));
    c = WzResource::load_texture(
        WzResource::ui->find(u"Basic.img/ComboBox/disabled/1"));
    r = WzResource::load_texture(
        WzResource::ui->find(u"Basic.img/ComboBox/disabled/2"));
  } else {
    auto mouse_pos = Window::mouse_pos;
    SDL_FRect pos_r;
    pos_r.x = x;
    pos_r.y = y;
    pos_r.w = cm.w;
    pos_r.h = 17;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_r)) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        l = WzResource::load_texture(
            WzResource::ui->find(u"Basic.img/ComboBox/normal/0"));
        c = WzResource::load_texture(
            WzResource::ui->find(u"Basic.img/ComboBox/normal/1"));
        r = WzResource::load_texture(
            WzResource::ui->find(u"Basic.img/ComboBox/normal/2"));
      } else {
        l = WzResource::load_texture(
            WzResource::ui->find(u"Basic.img/ComboBox/normal/0"));
        c = WzResource::load_texture(
            WzResource::ui->find(u"Basic.img/ComboBox/normal/1"));
        r = WzResource::load_texture(
            WzResource::ui->find(u"Basic.img/ComboBox/mouseOver/2"));
      }
    }
  }
  SDL_FRect pos_rect;
  pos_rect.x = x;
  pos_rect.y = y;
  pos_rect.w = l->w;
  pos_rect.h = l->h;
  SDL_RenderTexture(Window::renderer, l, nullptr, &pos_rect);
  pos_rect.x += l->w;
  pos_rect.w = cm.w - l->w - r->w;
  SDL_RenderTextureTiled(Window::renderer, c, nullptr, 1, &pos_rect);
  pos_rect.x += pos_rect.w;
  pos_rect.w = r->w;
  SDL_RenderTexture(Window::renderer, r, nullptr, &pos_rect);
  Freetype::load_size(12);
  Freetype::load_color(0, 0, 0, 255);
  Freetype::draw_line(cm.selected, x + 10, pos_rect.y);
  if (cm.open) {
    auto lh = Freetype::load_lh();
    auto h = cm.selects.size() * lh * 1.3;
    SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 255);
    SDL_FRect pos_rr;
    pos_rr.x = x;
    pos_rr.y = y + l->h;
    pos_rr.w = pos_rect.w;
    pos_rr.h = pos_rect.h;
    SDL_RenderRect(Window::renderer, &pos_rr);
    SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(Window::renderer, &pos_rr);
    auto base_y = pos_rr.y;
    for (int i = 0; i < cm.selects.size(); i++) {
      auto text = cm.selects[i];
      Freetype::draw_line(text, x + 10, base_y);
      base_y += lh * 1.3;
    }
  }
}

bool ComboBoxUiSystem::click(int x, int y, ComboBox cm, bool top,
                                bool disable) {
  return false;
}