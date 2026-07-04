#include "npc_render_system.h"
#include "SDL3/SDL_rect.h"
#include "nametag_render_system.h"
#include "src/client/game/game_nametag.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/system/render/chatballoon_render_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <ranges>

void npc_render_system::render_nametag(game_npc &g_npc) {

  auto npc_name = npc_game_instance::load_npc_text(g_npc.id, u"name");
  if (npc_name != u"") {
    game_nametag n;
    n.color = {255, 205, 0, 255};
    n.path = u"";
    n.pos = {0, 0};
    n.size = 13;
    n.text = npc_name;
    nametag_render_system::render(n, g_npc.pos);
  }

  auto npc_func = npc_game_instance::load_npc_text(g_npc.id, u"func");
  if (npc_func != u"") {
    game_nametag n;
    n.color = {255, 205, 0, 255};
    n.path = u"";
    n.pos = {0, 18};
    n.size = 13;
    n.text = npc_func;
    nametag_render_system::render(n, g_npc.pos);
  }
}

void npc_render_system::render_chatballoon(game_npc &g_npc) {
  if (!g_npc.chatballoon.has_value()) {
    return;
  }
  auto chatballoon = g_npc.chatballoon.value();
  auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);
  if (npc_node->type == wz::Type::UOL) {
    npc_node = static_cast<wz::Property<wz::WzUOL> *>(npc_node)->get_uol();
  }

  auto texture = wz_resource::load_texture(npc_node);
  auto origin = wz_resource::load_fpoint(npc_node->get_child(u"origin"));
  freetype::load_size(chatballoon.size);
  SDL_FPoint pos{
      .x = g_npc.pos.x,
      .y = g_npc.pos.y - origin.y - texture->h / 8,
  };
  chatballoon_render_system::render(chatballoon, pos);
}

bool npc_render_system::render_npc(game_npc &g_npc) {
  auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);
  if (npc_node->type == wz::Type::UOL) {
    npc_node = static_cast<wz::Property<wz::WzUOL> *>(npc_node)->get_uol();
  }

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
  render_nametag(g_npc);
  return true;
}