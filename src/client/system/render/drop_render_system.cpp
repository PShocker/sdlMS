#include "drop_render_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <string>

bool DropRenderSystem::Render(GameDrop &g_drop) {
  SDL_Texture *icon;
  SDL_FPoint o;
  switch (g_drop.data->type) {
  case ItemEnum::kEquip: {
  case ItemEnum::kDeco:
    auto equip_info = EquipGameInstance::LoadEquipInfo(g_drop.data->id);
    icon = WzResource::LoadTexture(equip_info->get_child(u"iconRaw"));
    o = WzResource::LoadFpoint(
        equip_info->get_child(u"iconRaw")->get_child(u"origin"));
    break;
  }
  case ItemEnum::kConsume:
  case ItemEnum::kEtc:
  case ItemEnum::kInstall: {
    auto item_num = ItemGameInstance::LoadItemNum(g_drop.data);
    auto item_info =
        ItemGameInstance::LoadItemInfo(g_drop.data->id, item_num);
    if (item_info->get_child(u"iconRaw")) {
      item_info = item_info->get_child(u"iconRaw");
      icon = WzResource::LoadTexture(item_info);
      o = WzResource::LoadFpoint(item_info->get_child(u"origin"));
    } else {
      auto now = Window::dt_now;
      auto sum = item_info->children_count() * 100;
      auto offset = now % sum; // 取余，得到周期内偏移
      auto i = offset / 100;
      item_info = item_info->get_child(std::to_string(i));
      icon = WzResource::LoadTexture(item_info);
      o = WzResource::LoadFpoint(item_info->get_child(u"origin"));
    }
    break;
  }
  case ItemEnum::kCash: {
    break;
  }
  }
  SDL_FRect pos_rect{
      g_drop.pos.x - o.x,
      g_drop.pos.y - o.y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  auto &camera = CameraGameInstance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_FPoint center{(float)icon->w / 2, (float)icon->h / 2};
    SDL_SetTextureAlphaMod(icon, g_drop.alpha);
    SDL_RenderTextureRotated(Window::renderer, icon, nullptr, &pos_rect,
                             g_drop.rotate, &center, (SDL_FlipMode)0);
  }
  return true;
}