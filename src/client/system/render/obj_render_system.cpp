#include "obj_render_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

bool obj_render_system::render(game_obj &g_obj) {
  auto obj_node = wz_resource::map->find(g_obj.path);
  auto index = std::to_string(g_obj.ani_index);
  obj_node = obj_node->get_child(index);
  auto texture = wz_resource::load_texture(obj_node);
    auto v =
      static_cast<wz::Property<wz::WzVec2D> *>(obj_node->get_child(u"origin"))
          ->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};
  SDL_FRect pos_rect = {
      .x = g_obj.pos.x - origin.x,
      .y = g_obj.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureAlphaMod(texture, g_obj.alpha);
    auto flip = g_obj.flip;
    SDL_RenderTextureRotated(window::renderer, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)flip);
  }

  return true;
}