#include "npc_render_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/render/chatballoon_render_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <ranges>

void npc_render_system::render_nametag(const std::u16string &text,
                                       SDL_FPoint p) {
  const auto &camera = camera_game_instance::camera;
  freetype::load_aligned(true);
  freetype::load_size(13);
  auto w = freetype::load_w(text);
  auto h = freetype::load_lh();

  auto x = p.x;
  auto y = p.y;
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
    freetype::load_color(255, 205, 0, 255);
    freetype::draw_line(text, x - camera.x - w / 2, y - camera.y);
  }
  return;
}

void npc_render_system::render_nametag(game_npc &g_npc) {
  auto npc_name = npc_game_instance::load_npc_text(g_npc.id, u"name");
  if (npc_name != u"") {
    render_nametag(npc_name, {g_npc.pos.x, g_npc.pos.y});
  }
  auto npc_func = npc_game_instance::load_npc_text(g_npc.id, u"func");
  if (npc_func != u"") {
    render_nametag(npc_func, {g_npc.pos.x, g_npc.pos.y + 18});
  }
}

void npc_render_system::render_chatballoon(game_npc &g_npc) {
  if (!g_npc.chatballoon.has_value()) {
    return;
  }
  auto quests = quest_game_instance::load_npc_quest(g_npc.id);
  if (!quests.empty()) {
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
    SDL_RenderTextureRotated(window::renderer, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)g_npc.flip);
  }

  return true;
}

void npc_render_system::render_quest(game_npc &g_npc) {
  auto quests = quest_game_instance::load_npc_quest(g_npc.id);
  if (!quests.empty()) {
    auto node = npc_game_instance::load_quest_node(g_npc);
    auto origin = wz_resource::load_fpoint(node->get_child(u"origin"));
    auto texture = wz_resource::load_texture(node);
    auto &camera = camera_game_instance::camera;
    auto npc_rect = npc_game_instance::load_rect(g_npc);
    auto pos_rect = npc_game_instance::load_quest_rect(g_npc).value();
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
}

bool npc_render_system::render(game_npc &g_npc) {
  render_npc(g_npc);
  render_nametag(g_npc);
  render_quest(g_npc);
  return true;
}