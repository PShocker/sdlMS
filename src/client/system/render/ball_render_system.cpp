#include "ball_render_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <string>

bool ball_render_system::render(game_ball &g_ball) {
  auto now = window::dt_time;
  if (g_ball.delay >= now) {
    return false;
  }
  SDL_Texture *t;
  wz::Node *n = nullptr;
  if (wz_resource::skill->find(g_ball.path)) {
    n = wz_resource::skill->find(g_ball.path);
  } else {
    n = wz_resource::item->find(g_ball.path);
  }
  auto index = std::to_string(g_ball.ani_index);
  auto texture_node = n->get_child(index);
  t = wz_resource::load_texture(texture_node);
  auto v = static_cast<wz::Property<wz::WzVec2D> *>(
               texture_node->get_child(u"origin"))
               ->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};
  SDL_FRect pos_rect = {
      .x = g_ball.pos.x - origin.x,
      .y = g_ball.pos.y - origin.y,
      .w = static_cast<float>(t->w),
      .h = static_cast<float>(t->h),
  };
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    auto flip = g_ball.flip;
    SDL_RenderTextureRotated(window::renderer, t, nullptr, &pos_rect,
                             g_ball.rotate, nullptr, (SDL_FlipMode)flip);
  }
  return true;
}