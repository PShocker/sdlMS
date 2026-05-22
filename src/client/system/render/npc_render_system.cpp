#include "npc_render_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <ranges>

void npc_render_system::render_nametag(game_npc &g_npc) {
  auto view =
      g_npc.id | std::views::drop_while([](char16_t c) { return c == u'0'; });

  std::u16string result(view.begin(), view.end());
  auto str_node = wz_resource::string->find(result);
  const auto &camera = camera_game_instance::camera;

  if (auto name_node = str_node->get_child(u"name")) {
    auto name_str =
        static_cast<wz::Property<std::u16string> *>(name_node)->get();
    freetype::load_size(13);
    auto w = freetype::load_w(name_str);
    auto h = freetype::load_lh();
    freetype::load_color(255, 255, 255, 255);
    auto x = g_npc.pos.x;
    auto y = g_npc.pos.y;

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
      freetype::draw_line(name_str, x - camera.x - w / 2, y - camera.y);
    }
  }

  if (auto func_node = str_node->get_child(u"func")) {
    auto func_str =
        static_cast<wz::Property<std::u16string> *>(func_node)->get();
    freetype::load_size(13);
    auto w = freetype::load_w(func_str);
    auto h = freetype::load_lh();
    freetype::load_color(255, 255, 255, 255);
    auto x = g_npc.pos.x;
    auto y = g_npc.pos.y + 20;

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
      freetype::draw_line(func_str, x - camera.x - w / 2, y - camera.y);
    }
  }
}

void npc_render_system::render_chatballoon(game_npc &g_npc) {
  if (!g_npc.chatballoon.has_value()) {
    return;
  }
  auto chatballoon = g_npc.chatballoon.value();
  auto text_w = 100.0f;
  auto text_h = freetype::load_h(chatballoon.text, text_w);

  
}

bool npc_render_system::render(game_npc &g_npc) {
  auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);

  auto texture = wz_resource::load_texture(npc_node);

  auto v =
      static_cast<wz::Property<wz::WzVec2D> *>(npc_node->get_child(u"origin"))
          ->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};

  SDL_FRect pos_rect = {
      .x = g_npc.pos.x - origin.x,
      .y = g_npc.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureAlphaMod(texture, g_npc.alpha);
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }

  return true;
}