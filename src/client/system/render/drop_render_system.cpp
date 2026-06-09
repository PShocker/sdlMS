#include "drop_render_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

bool drop_render_system::render(game_drop &g_drop) {
  SDL_Texture *icon;
  SDL_FPoint origin;
  if (auto *equip = std::get_if<game_equip>(&g_drop.data)) {
    // 处理装备
    auto equip_info = equip_game_instance::load_equip_info(equip->id);
    icon = wz_resource::load_texture(equip_info->get_child(u"iconRaw"));
    origin = wz_resource::load_fpoint(
        equip_info->get_child(u"iconRaw")->get_child(u"origin"));

  } else if (auto *item = std::get_if<game_item>(&g_drop.data)) {
    // 处理物品
  }
  SDL_FRect pos_rect{
      g_drop.pos.x - origin.x,
      g_drop.pos.y - origin.y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_FPoint center{(float)icon->w / 2, (float)icon->h / 2};

    SDL_RenderTextureRotated(window::renderer, icon, nullptr, &pos_rect,
                             g_drop.rotate, &center, (SDL_FlipMode)0);
  }
  return true;
}