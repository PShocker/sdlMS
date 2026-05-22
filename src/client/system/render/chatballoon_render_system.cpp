#include "chatballoon_render_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

bool chatballoon_render_system::render(game_chatballoon &g_chatballoon,
                                       SDL_FPoint base) {
  auto path = g_chatballoon.path;
  auto node = wz_resource::ui->find(path);
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

    rect.x = (int)rect.x;
    rect.y = (int)rect.y + texture_nw->h;
    rect.w = texture_w->w;
    rect.h = result_h;
    SDL_RenderTextureTiled(window::renderer, texture_w, nullptr, 1, &rect);

    rect.x = (int)(r_w - texture_e->w);
    rect.w = texture_e->w;
    rect.h = result_h;
    SDL_RenderTextureTiled(window::renderer, texture_e, nullptr, 1, &rect);

    rect.x = (int)(r_x + texture_nw->w);
    rect.y = (int)(r_y);
    rect.w = result_w;
    rect.h = texture_n->h;
    SDL_RenderTextureTiled(window::renderer, texture_n, nullptr, 1, &rect);

    rect.x = (int)(r_x + texture_nw->w);
    rect.y = (int)(r_y + result_h);
    rect.w = result_w;
    rect.h = texture_s->h;
    SDL_RenderTextureTiled(window::renderer, texture_s, nullptr, 1, &rect);
  }
  return true;
}