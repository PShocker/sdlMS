#include "src/client/system/render/drop_render_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <string>

#include "wz/Property.h"
#include "wz/Wz.h"


namespace sdlms {

namespace {

wz::Node* ResolveIconNode(wz::Node* node) {
  if (node == nullptr) {
    return nullptr;
  }
  if (node->type == wz::Type::UOL) {
    node = static_cast<wz::Property<wz::WzUOL>*>(node)->get_uol();
  }
  if (node == nullptr) {
    return nullptr;
  }
  if (node->type == wz::Type::Canvas) {
    return node;
  }
  // 金币等 iconRaw 是文件夹，取内部 iconRaw 或按时间取一帧。
  if (auto* nested = node->get_child(u"iconRaw"); nested != nullptr) {
    return ResolveIconNode(nested);
  }
  const auto count = node->children_count();
  if (count == 0) {
    return nullptr;
  }
  auto index = (Window::dt_now_ / 100) % count;
  return ResolveIconNode(node->get_child(std::to_string(index)));
}

}  // namespace

bool DropRenderSystem::Render(GameDrop& g_drop) {
  if (g_drop.data->id.empty()) {
    return false;
  }
  SDL_Texture* icon = nullptr;
  switch (g_drop.data->type) {
    case ItemEnum::kEquip:
    case ItemEnum::kDeco: {
      auto equip_info = EquipGameInstance::LoadEquipInfo(g_drop.data->id);
      if (equip_info == nullptr) {
        return false;
      }
      auto icon_node = ResolveIconNode(equip_info->get_child(u"iconRaw"));
      if (icon_node == nullptr) {
        return false;
      }
      icon = WzResource::LoadTexture(icon_node);
      break;
    }
    case ItemEnum::kConsume:
    case ItemEnum::kEtc:
    case ItemEnum::kInstall: {
      auto item_num = ItemGameInstance::LoadItemNum(g_drop.data);
      auto item_info =
          ItemGameInstance::LoadItemInfo(g_drop.data->id, item_num);
      if (item_info == nullptr) {
        return false;
      }
      auto* icon_node = item_info->get_child(u"iconRaw");
      if (icon_node == nullptr) {
        icon_node = item_info;
      }
      icon_node = ResolveIconNode(icon_node);
      if (icon_node == nullptr) {
        return false;
      }
      icon = WzResource::LoadTexture(icon_node);
      break;
    }
    case ItemEnum::kCash: {
      // Cash drops have a different Item.wz layout; skip rendering rather than
      // dereferencing an invalid icon.
      return false;
    }
  }
  if (icon == nullptr) {
    return false;
  }
  g_drop.icon_w = static_cast<float>(icon->w);
  g_drop.icon_h = static_cast<float>(icon->h);
  // 095：普通掉落以底部中心为锚点；吸收动画以中心为锚点。
  const bool picking = g_drop.type == GameDrop::DropEnum::kPick;
  SDL_FRect pos_rect{
      g_drop.pos.x - g_drop.icon_w / 2,
      g_drop.pos.y - (picking ? g_drop.icon_h / 2 : g_drop.icon_h),
      g_drop.icon_w,
      g_drop.icon_h,
  };
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_FPoint center{g_drop.icon_w / 2, g_drop.icon_h / 2};
    SDL_SetTextureAlphaMod(icon, g_drop.alpha);
    SDL_RenderTextureRotated(Window::renderer_, icon, nullptr, &pos_rect,
                             g_drop.rotate, &center, (SDL_FlipMode)0);
  }
  return true;
}

}  // namespace sdlms
