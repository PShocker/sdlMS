#include "src/client/system/render/npc_render_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/render/chatballoon_render_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Property.h>
#include <wz/Wz.h>

#include <ranges>

namespace sdlms {

void NpcRenderSystem::RenderNametag(const std::u16string& text, SDL_FPoint p) {
  const auto& camera = CameraGameInstance::camera_;
  Freetype::LoadAligned(true);
  Freetype::LoadSize(13);
  auto w = Freetype::LoadW(text);
  auto h = Freetype::LoadLh();

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
    SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 178);
    SDL_RenderFillRect(Window::renderer_, &rect);
    Freetype::LoadColor(255, 205, 0, 255);
    Freetype::DrawLine(text, x - camera.x - w / 2, y - camera.y);
  }
  return;
}

void NpcRenderSystem::RenderNametag(GameNpc& g_npc) {
  // 095 hideName（MapleTV info/hideName=1）：摆设不显示名字/功能行。
  if (g_npc.maple_tv) {
    return;
  }
  auto npc_name = NpcGameInstance::LoadNpcText(g_npc.id, u"name");
  if (npc_name != u"") {
    RenderNametag(npc_name, {g_npc.pos.x, g_npc.pos.y});
  }
  auto npc_func = NpcGameInstance::LoadNpcText(g_npc.id, u"func");
  if (npc_func != u"") {
    RenderNametag(npc_func, {g_npc.pos.x, g_npc.pos.y + 18});
  }
}

void NpcRenderSystem::RenderChatballoon(GameNpc& g_npc) {
  if (!g_npc.chatballoon.has_value()) {
    return;
  }
  // 095：只有实际显示任务图标的 NPC 才隐藏说话气泡。
  if (NpcGameInstance::LoadQuestNode(g_npc) != nullptr) {
    return;
  }
  auto chatballoon = g_npc.chatballoon.value();
  auto npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  if (npc_node == nullptr) {
    return;
  }
  auto action_node = npc_node->get_child(g_npc.action);
  if (action_node == nullptr) {
    return;
  }

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);
  if (npc_node == nullptr) {
    return;
  }
  if (npc_node->type == wz::Type::UOL) {
    npc_node = static_cast<wz::Property<wz::WzUOL>*>(npc_node)->get_uol();
  }
  if (npc_node == nullptr) {
    return;
  }

  auto texture = WzResource::LoadTexture(npc_node);
  auto origin = WzResource::LoadFpoint(npc_node->get_child(u"origin"));
  Freetype::LoadSize(chatballoon.size);
  SDL_FPoint pos{
      .x = g_npc.pos.x,
      .y = g_npc.pos.y - origin.y - texture->h / 8,
  };
  ChatballoonRenderSystem::Render(chatballoon, pos);
}

// 095 CNpc::Init 的 bMapleTV 分支：模板立绘是 1x1 占位，外观全部由
// UI/MapleTV.img 绘制——机身循环 TVon 动画（240x90，origin=(0,89)），
// 广告 TVmedia/<段>/<帧>（240x180 大屏）画在 pos+(adX,adY)，两路独立
// 计时（帧推进在 NpcLogicSystem::RunTvAnimate）。
bool RenderMapleTV(GameNpc& g_npc) {
  auto& camera = CameraGameInstance::camera_;
  auto draw_frame = [&camera, &g_npc](wz::Node* frame, SDL_FPoint at) {
    if (frame == nullptr || frame->get_child(u"origin") == nullptr) {
      return;
    }
    auto texture = WzResource::LoadTexture(frame);
    auto origin = WzResource::LoadFpoint(frame->get_child(u"origin"));
    SDL_FRect pos_rect{
        at.x - origin.x,
        at.y - origin.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
      pos_rect.x -= camera.x;
      pos_rect.y -= camera.y;
      SDL_RenderTextureRotated(Window::renderer_, texture, nullptr,
                               &pos_rect, 0, nullptr,
                               (SDL_FlipMode)g_npc.flip);
    }
  };
  // 机身：TVon 动画帧索引沿用 g_npc.ani_index（与普通 NPC 相同的推进
  // 节奏，由 RunTvAnimate 维护）。
  if (auto* tv_on = NpcGameInstance::LoadMapleTVNode(u"TVon");
      tv_on != nullptr) {
    auto index = std::to_string(g_npc.ani_index);
    draw_frame(tv_on->get_child(index), g_npc.pos);
  }
  // 广告：TVmedia/<段>/<帧> 于 (pos + adX, adY)。
  std::u16string media = u"TVmedia/" + [&] {
    auto s = std::to_string(g_npc.tv_media);
    return std::u16string{s.begin(), s.end()};
  }();
  if (auto* seg = NpcGameInstance::LoadMapleTVNode(media); seg != nullptr) {
    auto index = std::to_string(g_npc.tv_ani_index);
    draw_frame(seg->get_child(index),
               {g_npc.pos.x + static_cast<float>(g_npc.tv_ad_x),
                g_npc.pos.y + static_cast<float>(g_npc.tv_ad_y)});
  }
  return true;
}

bool NpcRenderSystem::RenderNpc(GameNpc& g_npc) {
  // MapleTV 摆设：不画 1x1 立绘，改画机身 + 广告。
  if (g_npc.maple_tv) {
    return RenderMapleTV(g_npc);
  }
  auto npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  if (npc_node == nullptr) {
    return false;
  }
  auto action_node = npc_node->get_child(g_npc.action);
  if (action_node == nullptr) {
    return false;
  }

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);
  if (npc_node == nullptr) {
    return false;
  }
  if (npc_node->type == wz::Type::UOL) {
    npc_node = static_cast<wz::Property<wz::WzUOL>*>(npc_node)->get_uol();
  }
  if (npc_node == nullptr) {
    return false;
  }

  auto texture = WzResource::LoadTexture(npc_node);
  auto origin = WzResource::LoadFpoint(npc_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = g_npc.pos.x - origin.x,
      .y = g_npc.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureAlphaMod(texture, g_npc.alpha);
    SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)g_npc.flip);
  }

  return true;
}

void NpcRenderSystem::RenderQuest(GameNpc& g_npc) {
  auto node = NpcGameInstance::LoadQuestNode(g_npc);
  if (node != nullptr) {
    auto origin = WzResource::LoadFpoint(node->get_child(u"origin"));
    auto texture = WzResource::LoadTexture(node);
    auto& camera = CameraGameInstance::camera_;
    auto pos_rect = NpcGameInstance::LoadQuestRect(g_npc).value();
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

bool NpcRenderSystem::Render(GameNpc& g_npc) {
  RenderNpc(g_npc);
  RenderNametag(g_npc);
  RenderQuest(g_npc);
  RenderChatballoon(g_npc);
  // 气泡由 SceneSystemInstance 在场景全部绘制完后再统一画一遍，
  // 保证它盖在前景 / 其他对象之上，这里不能再画第二次。
  return true;
}

}  // namespace sdlms
