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

bool DropRenderSystem::Render(GameDrop& g_drop) {
  if (g_drop.data->id.empty()) {
    return false;
  }
  SDL_Texture* icon = nullptr;
  SDL_FPoint o{};
  switch (g_drop.data->type) {
    case ItemEnum::kEquip:
    case ItemEnum::kDeco: {
      auto equip_info = EquipGameInstance::LoadEquipInfo(g_drop.data->id);
      if (equip_info == nullptr) {
        return false;
      }
      auto icon_node = equip_info->get_child(u"iconRaw");
      if (icon_node == nullptr) {
        return false;
      }
      if (icon_node->type == wz::Type::UOL) {
        icon_node = static_cast<wz::Property<wz::WzUOL>*>(icon_node)->get_uol();
      }
      if (icon_node == nullptr) {
        return false;
      }
      icon = WzResource::LoadTexture(icon_node);
      if (icon == nullptr) {
        return false;
      }
      if (auto origin = icon_node->get_child(u"origin");
          origin != nullptr) {
        o = WzResource::LoadFpoint(origin);
      }
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
      wz::Node* icon_node = item_info->get_child(u"iconRaw");
      if (icon_node != nullptr) {
        if (icon_node->type == wz::Type::UOL) {
          icon_node =
              static_cast<wz::Property<wz::WzUOL>*>(icon_node)->get_uol();
        }
        if (icon_node == nullptr) {
          return false;
        }
        icon = WzResource::LoadTexture(icon_node);
      } else {
        const auto count = item_info->children_count();
        if (count == 0) {
          return false;
        }
        auto now = Window::dt_now_;
        auto i = (now / 100) % count;
        icon_node = item_info->get_child(std::to_string(i));
        if (icon_node == nullptr) {
          return false;
        }
        icon = WzResource::LoadTexture(icon_node);
      }
      if (icon == nullptr) {
        return false;
      }
      if (icon_node != nullptr) {
        if (auto origin = icon_node->get_child(u"origin");
            origin != nullptr) {
          o = WzResource::LoadFpoint(origin);
        }
      }
      break;
    }
    case ItemEnum::kCash: {
      // Cash drops have a different Item.wz layout; skip rendering rather than
      // dereferencing an invalid icon.
      return false;
    }
  }
  SDL_FRect pos_rect{
      g_drop.pos.x - o.x,
      g_drop.pos.y - o.y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_FPoint center{(float)icon->w / 2, (float)icon->h / 2};
    SDL_SetTextureAlphaMod(icon, g_drop.alpha);
    SDL_RenderTextureRotated(Window::renderer_, icon, nullptr, &pos_rect,
                             g_drop.rotate, &center, (SDL_FlipMode)0);
  }
  return true;
}

}  // namespace sdlms
