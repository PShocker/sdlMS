#include "drop_render_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <string>

bool drop_render_system::render(game_drop &g_drop) {
  SDL_Texture *icon;
  SDL_FPoint o;
  switch (g_drop.data->type) {
  case item_enum::equip: {
  case item_enum::deco:
    auto equip_info = equip_game_instance::load_equip_info(g_drop.data->id);
    icon = wz_resource::load_texture(equip_info->get_child(u"iconRaw"));
    o = wz_resource::load_fpoint(
        equip_info->get_child(u"iconRaw")->get_child(u"origin"));
    break;
  }
  case item_enum::consume:
  case item_enum::etc:
  case item_enum::install: {
    auto item_num = item_game_instance::load_item_num(g_drop.data);
    auto item_info =
        item_game_instance::load_item_info(g_drop.data->id, item_num);
    if (item_info->get_child(u"iconRaw")) {
      item_info = item_info->get_child(u"iconRaw");
      icon = wz_resource::load_texture(item_info);
      o = wz_resource::load_fpoint(item_info->get_child(u"origin"));
    } else {
      auto now = window::dt_now;
      auto sum = item_info->children_count() * 100;
      auto offset = now % sum; // 取余，得到周期内偏移
      auto i = offset / 100;
      item_info = item_info->get_child(std::to_string(i));
      icon = wz_resource::load_texture(item_info);
      o = wz_resource::load_fpoint(item_info->get_child(u"origin"));
    }
    break;
  }
  case item_enum::cash: {
    break;
  }
  }
  SDL_FRect pos_rect{
      g_drop.pos.x - o.x,
      g_drop.pos.y - o.y,
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