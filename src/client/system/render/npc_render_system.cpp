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

void NpcRenderSystem::render_nametag(const std::u16string &text,
                                       SDL_FPoint p) {
  const auto &camera = CameraGameInstance::camera;
  Freetype::load_aligned(true);
  Freetype::load_size(13);
  auto w = Freetype::load_w(text);
  auto h = Freetype::load_lh();

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
    SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 178);
    SDL_RenderFillRect(Window::renderer, &rect);
    Freetype::load_color(255, 205, 0, 255);
    Freetype::draw_line(text, x - camera.x - w / 2, y - camera.y);
  }
  return;
}

void NpcRenderSystem::render_nametag(GameNpc &g_npc) {
  auto npc_name = NpcGameInstance::load_npc_text(g_npc.id, u"name");
  if (npc_name != u"") {
    render_nametag(npc_name, {g_npc.pos.x, g_npc.pos.y});
  }
  auto npc_func = NpcGameInstance::load_npc_text(g_npc.id, u"func");
  if (npc_func != u"") {
    render_nametag(npc_func, {g_npc.pos.x, g_npc.pos.y + 18});
  }
}

void NpcRenderSystem::render_chatballoon(GameNpc &g_npc) {
  if (!g_npc.chatballoon.has_value()) {
    return;
  }
  auto quests = QuestGameInstance::load_npc_quest(g_npc.id);
  if (!quests.empty()) {
    return;
  }
  auto chatballoon = g_npc.chatballoon.value();
  auto npc_node = NpcGameInstance::load_link_npc_node(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);
  if (npc_node->type == wz::Type::UOL) {
    npc_node = static_cast<wz::Property<wz::WzUOL> *>(npc_node)->get_uol();
  }

  auto texture = WzResource::load_texture(npc_node);
  auto origin = WzResource::load_fpoint(npc_node->get_child(u"origin"));
  Freetype::load_size(chatballoon.size);
  SDL_FPoint pos{
      .x = g_npc.pos.x,
      .y = g_npc.pos.y - origin.y - texture->h / 8,
  };
  ChatballoonRenderSystem::render(chatballoon, pos);
}

bool NpcRenderSystem::render_npc(GameNpc &g_npc) {
  auto npc_node = NpcGameInstance::load_link_npc_node(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);
  if (npc_node->type == wz::Type::UOL) {
    npc_node = static_cast<wz::Property<wz::WzUOL> *>(npc_node)->get_uol();
  }

  auto texture = WzResource::load_texture(npc_node);
  auto origin = WzResource::load_fpoint(npc_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = g_npc.pos.x - origin.x,
      .y = g_npc.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto &camera = CameraGameInstance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureAlphaMod(texture, g_npc.alpha);
    SDL_RenderTextureRotated(Window::renderer, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)g_npc.flip);
  }

  return true;
}

void NpcRenderSystem::render_quest(GameNpc &g_npc) {
  auto node = NpcGameInstance::load_quest_node(g_npc);
  if (node != nullptr) {
    auto origin = WzResource::load_fpoint(node->get_child(u"origin"));
    auto texture = WzResource::load_texture(node);
    auto &camera = CameraGameInstance::camera;
    auto pos_rect = NpcGameInstance::load_quest_rect(g_npc).value();
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
  }
}

bool NpcRenderSystem::render(GameNpc &g_npc) {
  render_npc(g_npc);
  render_nametag(g_npc);
  render_quest(g_npc);
  return true;
}