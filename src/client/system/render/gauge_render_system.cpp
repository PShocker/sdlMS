#include "gauge_render_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

bool gauge_render_system::render(SDL_FPoint pos, game_gauge &g) {
  static auto back = wz_resource::load_texture(
      wz_resource::ui->find(u"PartyHP.img/GaugeBar2/graduation"));
  const auto w = 200;

  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x - back->w / 2),
      static_cast<float>((int)pos.y),
      static_cast<float>(back->w),
      static_cast<float>(back->h),
  };
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= (int)camera.x;
    pos_rect.y -= (int)camera.y;
    SDL_RenderTexture(window::renderer, back, nullptr, &pos_rect);

    auto x = (int)pos_rect.x + 3;
    auto y = (int)pos_rect.y + 3;
    auto w = (pos_rect.w - 6);
    auto h = 4;
    SDL_SetRenderDrawColor(window::renderer, 255, 0, 0, 255);
    pos_rect.x = x;
    pos_rect.y = y;
    pos_rect.w = w * g.hp_percent;
    pos_rect.h = h;
    SDL_RenderFillRect(window::renderer, &pos_rect);

    SDL_SetRenderDrawColor(window::renderer, 0, 255, 0, 255);
    pos_rect.w = w * g.hp_percent_now;
    SDL_RenderFillRect(window::renderer, &pos_rect);
  }

  return true;
}