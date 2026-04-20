#include "mob_render_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

bool mob_render_system::render(game_mob &g_mob) {
  auto mob_node = mob_game_instance::load_link_mob_node(g_mob.id);

  auto action_node = mob_node->get_child(g_mob.action);

  auto index = std::to_string(g_mob.ani_index);
  mob_node = action_node->get_child(index);

  auto texture = wz_resource::load_texture(mob_node);

  auto v = static_cast<wz::Property<wz::WzVec2D> *>(
               mob_node->get_child(u"origin"))
               ->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};

  SDL_FRect pos_rect = {
      .x = g_mob.pos.x - origin.x,
      .y = g_mob.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureAlphaMod(texture, g_mob.alpha);
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }

  return true;
}