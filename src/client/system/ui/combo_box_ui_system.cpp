#include "combo_box_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

void combo_box_ui_system::render(int x, int y, combo_box cm, bool top,
                                 bool disable) {
  SDL_Texture *l = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/ComboBox/normal/0"));
  SDL_Texture *c = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/ComboBox/normal/1"));
  SDL_Texture *r = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/ComboBox/normal/2"));
  if (disable) {
    l = wz_resource::load_texture(
        wz_resource::ui->find(u"Basic.img/ComboBox/disabled/0"));
    c = wz_resource::load_texture(
        wz_resource::ui->find(u"Basic.img/ComboBox/disabled/1"));
    r = wz_resource::load_texture(
        wz_resource::ui->find(u"Basic.img/ComboBox/disabled/2"));
  } else {
    auto mouse_pos = window::mouse_pos;
    SDL_FRect pos_r;
    pos_r.x = x;
    pos_r.y = y;
    pos_r.w = cm.w;
    pos_r.h = 17;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_r)) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        l = wz_resource::load_texture(
            wz_resource::ui->find(u"Basic.img/ComboBox/normal/0"));
        c = wz_resource::load_texture(
            wz_resource::ui->find(u"Basic.img/ComboBox/normal/1"));
        r = wz_resource::load_texture(
            wz_resource::ui->find(u"Basic.img/ComboBox/normal/2"));
      } else {
        l = wz_resource::load_texture(
            wz_resource::ui->find(u"Basic.img/ComboBox/normal/0"));
        c = wz_resource::load_texture(
            wz_resource::ui->find(u"Basic.img/ComboBox/normal/1"));
        r = wz_resource::load_texture(
            wz_resource::ui->find(u"Basic.img/ComboBox/mouseOver/2"));
      }
    }
  }
  SDL_FRect pos_rect;
  pos_rect.x = x;
  pos_rect.y = y;
  pos_rect.w = l->w;
  pos_rect.h = l->h;
  SDL_RenderTexture(window::renderer, l, nullptr, &pos_rect);
  pos_rect.x += l->w;
  pos_rect.w = cm.w - l->w - r->w;
  SDL_RenderTextureTiled(window::renderer, c, nullptr, 1, &pos_rect);
  pos_rect.x += pos_rect.w;
  pos_rect.w = r->w;
  SDL_RenderTexture(window::renderer, r, nullptr, &pos_rect);
  freetype::load_size(12);
  freetype::load_color(0, 0, 0, 255);
  freetype::draw_line(cm.selected, x + 10, pos_rect.y);
  if (cm.open) {
    auto lh = freetype::load_lh();
    auto h = cm.selects.size() * lh * 1.3;
    SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, 255);
    SDL_FRect pos_rr;
    pos_rr.x = x;
    pos_rr.y = y + l->h;
    pos_rr.w = pos_rect.w;
    pos_rr.h = pos_rect.h;
    SDL_RenderRect(window::renderer, &pos_rr);
    SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(window::renderer, &pos_rr);
    auto base_y = pos_rr.y;
    for (int i = 0; i < cm.selects.size(); i++) {
      auto text = cm.selects[i];
      freetype::draw_line(text, x + 10, base_y);
      base_y += lh * 1.3;
    }
  }
}

bool combo_box_ui_system::click(int x, int y, combo_box cm, bool top,
                                bool disable) {
  return false;
}