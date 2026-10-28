#include "chatballoon_render_system.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <numeric>
#include <optional>

std::optional<ChatballoonRenderSystem::BackgrndPad>
ChatballoonRenderSystem::RenderBackgrnd(GameChatballoon &g_chatballoon,
                                           SDL_FPoint base) {
  auto path = g_chatballoon.path;
  auto node = WzResource::ui_->find(u"ChatBalloon.img/" + path);
  auto texture_c = WzResource::LoadTexture(node->get_child(u"c"));
  auto texture_e = WzResource::LoadTexture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"e")));
  auto texture_ne = WzResource::LoadTexture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"ne")));
  auto texture_n = WzResource::LoadTexture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"n")));
  auto texture_nw = WzResource::LoadTexture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"nw")));
  auto texture_w = WzResource::LoadTexture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"w")));
  auto texture_sw = WzResource::LoadTexture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"sw")));
  auto texture_s = WzResource::LoadTexture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"s")));
  auto texture_se = WzResource::LoadTexture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"se")));
  auto texture_arrow = WzResource::LoadTexture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"arrow")));

  auto origin_c = WzResource::LoadFpoint(node->find(u"c/origin"));
  auto origin_e = WzResource::LoadFpoint(node->find(u"e/origin"));
  auto origin_ne = WzResource::LoadFpoint(node->find(u"ne/origin"));
  auto origin_n = WzResource::LoadFpoint(node->find(u"n/origin"));
  auto origin_nw = WzResource::LoadFpoint(node->find(u"nw/origin"));
  auto origin_w = WzResource::LoadFpoint(node->find(u"w/origin"));
  auto origin_sw = WzResource::LoadFpoint(node->find(u"sw/origin"));
  auto origin_s = WzResource::LoadFpoint(node->find(u"s/origin"));
  auto origin_se = WzResource::LoadFpoint(node->find(u"se/origin"));
  auto origin_arrow = WzResource::LoadFpoint(node->find(u"arrow/origin"));

  Freetype::LoadSize(g_chatballoon.size);
  auto text_w = g_chatballoon.w - (texture_nw->w + texture_ne->w);

  int a = (float)texture_c->w;
  int b = g_chatballoon.w;
  auto result_w = a * ((b + a - 1) / a); // 整数向上取整技巧

  int h = Freetype::LoadCh(g_chatballoon.text, result_w, 1.3);

  a = h;
  b = (float)texture_c->h;
  auto result_h = a * ((b + a - 1) / a); // 整数向上取整技巧

  SDL_FRect rect;
  rect.w = result_w + texture_nw->w + texture_ne->w;
  rect.h = result_h + texture_nw->h + texture_sw->h + texture_arrow->h;
  base.y -= rect.h;
  rect.x = base.x - rect.w / 2;
  rect.y = base.y;
  const auto &camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&rect, &camera)) {
    rect.x -= camera.x;
    rect.y -= camera.y;

    int r_x = rect.x;
    int r_y = rect.y;
    int r_w = rect.w;
    int r_h = rect.h;

    // 左上
    SDL_FPoint p_nw{static_cast<float>(r_x), static_cast<float>(r_y)};
    rect.x = (int)(p_nw.x);
    rect.y = (int)(p_nw.y);
    rect.w = texture_nw->w;
    rect.h = texture_nw->h;
    SDL_RenderTexture(Window::renderer_, texture_nw, nullptr, &rect);

    // 左下
    SDL_FPoint p_sw{
        r_x + (origin_nw.x - origin_sw.x),
        r_y + texture_nw->h + result_h - (origin_nw.y - origin_n.y),
    };
    rect.x = (int)(p_sw.x);
    rect.y = (int)(p_sw.y);
    rect.w = texture_sw->w;
    rect.h = texture_sw->h;
    SDL_RenderTexture(Window::renderer_, texture_sw, nullptr, &rect);

    // 右上
    SDL_FPoint p_ne{
        r_x + r_w - texture_ne->w - (origin_nw.x - origin_w.x),
        r_y + (origin_nw.y - origin_n.y),
    };
    rect.x = (int)(p_ne.x);
    rect.y = (int)(p_ne.y);
    rect.w = texture_ne->w;
    rect.h = texture_ne->h;
    SDL_RenderTexture(Window::renderer_, texture_ne, nullptr, &rect);

    // 右下
    SDL_FPoint p_se{
        p_ne.x,
        r_y + texture_nw->h + result_h - (origin_nw.y - origin_n.y),
    };
    rect.x = (int)(p_se.x);
    rect.y = (int)(p_se.y);
    rect.w = texture_se->w;
    rect.h = texture_se->h;
    SDL_RenderTexture(Window::renderer_, texture_se, nullptr, &rect);

    rect.x = (int)(r_x + texture_nw->w);
    rect.y = (int)(r_y + texture_nw->h);
    rect.w = p_ne.x - (p_nw.x + texture_nw->w);
    rect.h = p_sw.y - (p_nw.y + texture_nw->h);
    SDL_RenderTextureTiled(Window::renderer_, texture_c, nullptr, 1, &rect);

    // 竖着的两边
    rect.x = (int)(r_x + (origin_nw.x - origin_w.x));
    rect.y = (int)(p_nw.y + texture_nw->h);
    rect.w = texture_w->w;
    rect.h = p_sw.y - (p_nw.y + texture_nw->h);
    SDL_RenderTextureTiled(Window::renderer_, texture_w, nullptr, 1, &rect);

    rect.x = (int)(p_se.x);
    rect.y = (int)(p_ne.y + texture_ne->h);
    rect.w = texture_e->w;
    rect.h = p_se.y - (p_ne.y + texture_ne->h);
    SDL_RenderTextureTiled(Window::renderer_, texture_e, nullptr, 1, &rect);

    // 横着的两边
    rect.x = (int)(p_nw.x + texture_nw->w);
    rect.y = (int)(p_ne.y);
    rect.w = p_ne.x - (p_nw.x + texture_nw->w);
    rect.h = texture_n->h;
    SDL_RenderTextureTiled(Window::renderer_, texture_n, nullptr, 1, &rect);

    // 箭头
    SDL_FPoint p_arrow{
        r_x + r_w / 2 - (float)texture_arrow->w / 2,
        r_y + texture_nw->h + result_h - (origin_nw.y - origin_n.y),
    };
    rect.x = (int)(p_arrow.x);
    rect.y = (int)(p_arrow.y);
    rect.w = texture_arrow->w;
    rect.h = texture_arrow->h;
    SDL_RenderTexture(Window::renderer_, texture_arrow, nullptr, &rect);
    // 箭头两边
    rect.x = (int)(p_sw.x + texture_sw->w);
    rect.y = (int)(p_sw.y);
    rect.w = p_arrow.x - (p_sw.x + texture_sw->w) + origin_arrow.x;
    rect.h = texture_s->h;
    SDL_RenderTextureTiled(Window::renderer_, texture_s, nullptr, 1, &rect);

    rect.x = (int)(p_arrow.x + texture_arrow->w);
    rect.y = (int)(p_arrow.y);
    rect.w = p_se.x - (p_arrow.x + texture_arrow->w);
    rect.h = texture_s->h;
    SDL_RenderTextureTiled(Window::renderer_, texture_s, nullptr, 1, &rect);
    std::optional<int32_t> clr;
    if (auto clr_node = node->get_child(u"clr")) {
      clr = static_cast<wz::Property<int32_t> *>(clr_node)->get();
    }
    return BackgrndPad{
        static_cast<float>(r_x + texture_nw->w),
        static_cast<float>(r_y + texture_nw->h),
        static_cast<float>(r_w - texture_nw->w - texture_se->w),
        static_cast<float>(r_h - texture_nw->h - texture_se->h),
        clr,
    };
  }
  return std::nullopt;
}

bool ChatballoonRenderSystem::Render(GameChatballoon &g_chatballoon,
                                       SDL_FPoint base) {
  auto bacgrnd_pad = RenderBackgrnd(g_chatballoon, base);
  if (bacgrnd_pad.has_value()) {
    if (g_chatballoon.color.has_value()) {
      uint8_t a, r, g, b;
      a = g_chatballoon.color->a;
      r = g_chatballoon.color->r;
      g = g_chatballoon.color->g;
      b = g_chatballoon.color->b;
      Freetype::LoadColor(r, g, b, a);
    } else if (bacgrnd_pad->color.has_value()) {
      auto clr = bacgrnd_pad->color.value();
      uint8_t a, r, g, b;
      a = (clr >> 24) & 0xFF;
      r = (clr >> 16) & 0xFF;
      g = (clr >> 8) & 0xFF;
      b = clr & 0xFF;
      Freetype::LoadColor(r, g, b, a);
    }

    Freetype::LoadAligned(true);
    Freetype::LoadSize(g_chatballoon.size);
    Freetype::DrawCstr(g_chatballoon.text, bacgrnd_pad->x, bacgrnd_pad->y,
                        bacgrnd_pad->w, 1.3);
  }
  return true;
}