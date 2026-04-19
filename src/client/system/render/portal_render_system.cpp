#include "portal_render_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <string>

bool portal_render_system::render(game_portal &g_portal) {
  auto path = g_portal.path + g_portal.path2;

  auto portal_node = wz_resource::map->find(path);
  auto portal_pos = g_portal.pos;
  wz::Node *texture_node;
  if (portal_node->get_child(u"portalStart")) {
    // 三段式传送门
    auto self_character = character_game_instance::self;
    auto distance_x = (portal_pos.x - self_character.pos.x);
    auto distance_y = (portal_pos.y - self_character.pos.y);
    auto distance = distance_x * distance_x + distance_y * distance_y;
    if (distance <= 10000) {
      texture_node = portal_node->get_child(u"portalStart");
    } else if (distance <= 22500) {
      texture_node = portal_node->get_child(u"portalExit");
    } else {
      return false;
    }
  }
  auto index =
      std::to_string(g_portal.ani_index % texture_node->children_count());
  texture_node = portal_node->get_child(index);
  auto texture = wz_resource::load_texture(texture_node);
  auto v = static_cast<wz::Property<wz::WzVec2D> *>(
               texture_node->get_child(u"origin"))
               ->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};

  SDL_FRect pos_rect = {
      .x = portal_pos.x - origin.x,
      .y = portal_pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
  return true;
}