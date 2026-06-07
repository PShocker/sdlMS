#include "chatballoon_render_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

bool chatballoon_render_system::render(game_chatballoon &g_chatballoon,
                                       SDL_FPoint base) {
  auto path = g_chatballoon.path;
  auto node = wz_resource::ui->find(u"ChatBalloon.img/" + path);
  auto texture_c = wz_resource::load_texture(node->get_child(u"c"));
  auto texture_e = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"e")));
  auto texture_ne = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"ne")));
  auto texture_n = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"n")));
  auto texture_nw = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"nw")));
  auto texture_w = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"w")));
  auto texture_sw = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"sw")));
  auto texture_s = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"s")));
  auto texture_se = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"se")));
  auto texture_arrow = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"arrow")));

  auto origin_c = wz_resource::load_fpoint(node->find(u"c/origin"));
  auto origin_e = wz_resource::load_fpoint(node->find(u"e/origin"));
  auto origin_ne = wz_resource::load_fpoint(node->find(u"ne/origin"));
  auto origin_n = wz_resource::load_fpoint(node->find(u"n/origin"));
  auto origin_nw = wz_resource::load_fpoint(node->find(u"nw/origin"));
  auto origin_w = wz_resource::load_fpoint(node->find(u"w/origin"));
  auto origin_sw = wz_resource::load_fpoint(node->find(u"sw/origin"));
  auto origin_s = wz_resource::load_fpoint(node->find(u"s/origin"));
  auto origin_se = wz_resource::load_fpoint(node->find(u"se/origin"));
  auto origin_arrow = wz_resource::load_fpoint(node->find(u"arrow/origin"));

  freetype::load_size(g_chatballoon.size);
  auto h = freetype::load_h(g_chatballoon.text, g_chatballoon.w);

  auto a = (float)texture_c->w;
  auto b = g_chatballoon.w;
  auto result_w = a * ((b + a - 1) / a); // 整数向上取整技巧

  a = (float)texture_c->h;
  b = h;
  auto result_h = a * ((b + a - 1) / a); // 整数向上取整技巧

  SDL_FRect rect;
  rect.w = result_w + texture_nw->w + texture_ne->w;
  rect.h = result_h + texture_nw->h + texture_sw->h + texture_arrow->h;
  rect.x = base.x - rect.w / 2;
  rect.y = base.y;
  const auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&rect, &camera)) {
    rect.x -= camera.x;
    rect.y -= camera.y;

    auto r_x = rect.x;
    auto r_y = rect.y;
    auto r_w = rect.w;
    auto r_h = rect.h;

    // 左上
    SDL_FPoint p_nw{r_x, r_y};
    rect.x = int(p_nw.x);
    rect.y = int(p_nw.y);
    rect.w = texture_nw->w;
    rect.h = texture_nw->h;
    SDL_RenderTexture(window::renderer, texture_nw, nullptr, &rect);

    // 左下
    SDL_FPoint p_sw{
        r_x + (origin_nw.x - origin_sw.x),
        r_y + texture_nw->h + result_h - (origin_nw.y - origin_n.y),
    };
    rect.x = int(p_sw.x);
    rect.y = int(p_sw.y);
    rect.w = texture_sw->w;
    rect.h = texture_sw->h;
    SDL_RenderTexture(window::renderer, texture_sw, nullptr, &rect);

    // 右上
    SDL_FPoint p_ne{
        r_x + r_w - texture_ne->w - (origin_nw.x - origin_w.x),
        r_y + (origin_nw.y - origin_n.y),
    };
    rect.x = int(p_ne.x);
    rect.y = int(p_ne.y);
    rect.w = texture_ne->w;
    rect.h = texture_ne->h;
    SDL_RenderTexture(window::renderer, texture_ne, nullptr, &rect);

    // 右下
    SDL_FPoint p_se{
        p_ne.x,
        r_y + texture_nw->h + result_h - (origin_nw.y - origin_n.y),
    };
    rect.x = int(p_se.x);
    rect.y = int(p_se.y);
    rect.w = texture_se->w;
    rect.h = texture_se->h;
    SDL_RenderTexture(window::renderer, texture_se, nullptr, &rect);

    rect.x = int(r_x + texture_nw->w);
    rect.y = int(r_y + texture_nw->h);
    rect.w = p_ne.x - (p_nw.x + texture_nw->w);
    rect.h = p_sw.y - (p_nw.y + texture_nw->h);
    SDL_RenderTextureTiled(window::renderer, texture_c, nullptr, 1, &rect);

    // 竖着的两边
    rect.x = int(r_x + (origin_nw.x - origin_w.x));
    rect.y = int(p_nw.y + texture_nw->h);
    rect.w = texture_w->w;
    rect.h = p_sw.y - (p_nw.y + texture_nw->h);
    SDL_RenderTextureTiled(window::renderer, texture_w, nullptr, 1, &rect);

    rect.x = int(p_se.x);
    rect.y = int(p_ne.y + texture_ne->h);
    rect.w = texture_e->w;
    rect.h = p_se.y - (p_ne.y + texture_ne->h);
    SDL_RenderTextureTiled(window::renderer, texture_e, nullptr, 1, &rect);

    // 横着的两边
    rect.x = int(p_nw.x + texture_nw->w);
    rect.y = int(p_ne.y);
    rect.w = p_ne.x - (p_nw.x + texture_nw->w);
    rect.h = texture_n->h;
    SDL_RenderTextureTiled(window::renderer, texture_n, nullptr, 1, &rect);

    // 箭头
    SDL_FPoint p_arrow{
        r_x + r_w / 2 - (float)texture_arrow->w / 2,
        r_y + texture_nw->h + result_h - (origin_nw.y - origin_n.y),
    };
    rect.x = int(p_arrow.x);
    rect.y = int(p_arrow.y);
    rect.w = texture_arrow->w;
    rect.h = texture_arrow->h;
    SDL_RenderTexture(window::renderer, texture_arrow, nullptr, &rect);
    // 箭头两边
    rect.x = int(p_sw.x + texture_sw->w);
    rect.y = int(p_sw.y);
    rect.w = p_arrow.x - (p_sw.x + texture_sw->w) + origin_arrow.x;
    rect.h = texture_s->h;
    SDL_RenderTextureTiled(window::renderer, texture_s, nullptr, 1, &rect);

    rect.x = int(p_arrow.x + texture_arrow->w);
    rect.y = int(p_arrow.y);
    rect.w = p_se.x - (p_arrow.x + texture_arrow->w);
    rect.h = texture_s->h;
    SDL_RenderTextureTiled(window::renderer, texture_s, nullptr, 1, &rect);

    if (g_chatballoon.color.has_value()) {
      uint8_t a, r, g, b;
      a = g_chatballoon.color->a;
      r = g_chatballoon.color->r;
      g = g_chatballoon.color->g;
      b = g_chatballoon.color->b;
      freetype::load_color(r, g, b, a);
    } else {
      auto clr =
          static_cast<wz::Property<int32_t> *>(node->get_child(u"clr"))->get();
      uint8_t a, r, g, b;
      a = (clr >> 24) & 0xFF;
      r = (clr >> 16) & 0xFF;
      g = (clr >> 8) & 0xFF;
      b = clr & 0xFF;
      freetype::load_color(r, g, b, a);
    }

    freetype::load_aligned(true);
    freetype::draw_str(g_chatballoon.text, r_x + texture_nw->w,
                       r_y + texture_nw->h, g_chatballoon.w);
    freetype::load_aligned(false);
  }
  return true;
}