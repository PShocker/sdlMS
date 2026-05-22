#include "nametag_render_system.h"
#include "src/client/game/game_nametag.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

bool nametag_render_system::render(game_nametag &g_nametag, SDL_FPoint base) {
  const auto &camera = camera_game_instance::camera;
  freetype::load_aligned(true);
  freetype::load_size(g_nametag.size);
  auto w = freetype::load_w(g_nametag.text);
  auto h = freetype::load_lh();
  freetype::load_color(g_nametag.color.r, g_nametag.color.g, g_nametag.color.b,
                       g_nametag.color.a);
  auto x = base.x + g_nametag.pos.x;
  auto y = base.y + g_nametag.pos.y;
  if (g_nametag.path == u"") {
    SDL_FRect rect;
    rect.w = w + 4;
    rect.h = h + 4;
    rect.x = x - rect.w / 2;
    rect.y = y;
    if (SDL_HasRectIntersectionFloat(&rect, &camera)) {
      rect.x -= camera.x;
      rect.y -= camera.y;
      SDL_SetRenderDrawBlendMode(window::renderer, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, 178);
      SDL_RenderFillRect(window::renderer, &rect);
      freetype::draw_line(g_nametag.text, x - camera.x - w / 2, y - camera.y);
    }
  } else {
    static auto nametag_node = wz_resource::ui->find(u"NameTag.img");
    auto texture_node = nametag_node->get_child(g_nametag.path);
    auto texture_w = wz_resource::load_texture(texture_node->get_child("w"));
    auto texture_c = wz_resource::load_texture(texture_node->get_child("c"));
    auto texture_e = wz_resource::load_texture(texture_node->get_child("e"));

    auto ow = wz_resource::load_fpoint(texture_node->find(u"w/origin"));
    auto oc = wz_resource::load_fpoint(texture_node->find(u"c/origin"));

    auto a = (float)texture_c->w;
    auto b = w;
    auto result = a * ((b + a - 1) / a); // 整数向上取整技巧

    SDL_FRect rect;
    rect.w = result + texture_w->w + texture_e->w;
    rect.h = texture_c->h;
    rect.x = x - rect.w / 2;
    rect.y = y;
    if (SDL_HasRectIntersectionFloat(&rect, &camera)) {
      rect.x -= camera.x;
      rect.y -= camera.y;
      rect.x = (int)rect.x;
      rect.y = (int)(rect.y - (ow.y - oc.y));
      rect.w = texture_w->w;
      rect.h = texture_w->h;
      SDL_RenderTexture(window::renderer, texture_w, nullptr, &rect);
      rect.x += (int)texture_w->w;
      rect.y = (int)(y - camera.y);
      rect.w = result;
      rect.h = texture_c->h;
      SDL_RenderTextureTiled(window::renderer, texture_c, nullptr, 1, &rect);
      rect.x += result;
      rect.w = texture_e->w;
      SDL_RenderTexture(window::renderer, texture_e, nullptr, &rect);
      auto clr =
          static_cast<wz::Property<int32_t> *>(texture_node->get_child(u"clr"))
              ->get();
      uint8_t a, r, g, b;
      a = (clr >> 24) & 0xFF;
      r = (clr >> 16) & 0xFF;
      g = (clr >> 8) & 0xFF;
      b = clr & 0xFF;
      freetype::load_color(r, g, b, a);
      freetype::draw_line(g_nametag.text, rect.x - result, y - camera.y);
    }
  }
  freetype::load_aligned(false);
  return true;
}