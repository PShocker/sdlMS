#include "gauge_render_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

bool gauge_render_system::render(SDL_FPoint pos, game_gauge &g,
                                 int64_t new_val) {
  static auto node = wz_resource::ui->find(u"BossMobHP.img/MobGage");
  static auto back2 = wz_resource::load_texture(node->get_child(u"backgrnd2"));
  static auto back3 = wz_resource::load_texture(node->get_child(u"backgrnd3"));
  static auto back4 = wz_resource::load_texture(node->get_child(u"backgrnd4"));
  const auto w = 200;

  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(back2->w),
      static_cast<float>(back2->h),
  };
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(window::renderer, back2, nullptr, &pos_rect);
  }
  pos_rect = {
      pos.x + back2->w,
      pos.y,
      static_cast<float>(w - back2->w - back4->w),
      static_cast<float>(back2->h),
  };
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(window::renderer, back3, nullptr, &pos_rect);
  }
  pos_rect = {
      pos.x + w - back4->w,
      pos.y,
      static_cast<float>(back4->w),
      static_cast<float>(back4->h),
  };
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(window::renderer, back4, nullptr, &pos_rect);
  }
  return true;
}