#include "npc_render_system.h"
#include "nametag_render_system.h"
#include "src/client/game/game_nametag.h"
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

  if (auto name_node = str_node->get_child(u"name")) {
    auto name_str =
        static_cast<wz::Property<std::u16string> *>(name_node)->get();
    game_nametag n;
    n.color = {255, 255, 255, 255};
    n.path = u"";
    n.pos = {0, 0};
    n.size = 13;
    n.text = name_str;
    nametag_render_system::render(n, g_npc.pos);
  }

  if (auto func_node = str_node->get_child(u"func")) {
    auto func_str =
        static_cast<wz::Property<std::u16string> *>(func_node)->get();
    game_nametag n;
    n.color = {255, 255, 255, 255};
    n.path = u"";
    n.pos = {0, 20};
    n.size = 13;
    n.text = func_str;
    nametag_render_system::render(n, g_npc.pos);
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

bool npc_render_system::render_npc(game_npc &g_npc) {
  auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);

  auto texture = wz_resource::load_texture(npc_node);
  auto origin = wz_resource::load_fpoint(npc_node->get_child(u"origin"));
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

bool npc_render_system::render(game_npc &g_npc) {
  render_npc(g_npc);
  render_chatballoon(g_npc);
  render_nametag(g_npc);
  return true;
}