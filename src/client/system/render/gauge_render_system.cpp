#include "src/client/system/render/gauge_render_system.h"
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
namespace sdlms {


bool GaugeRenderSystem::RenderMob(SDL_FPoint pos, GameGauge &g) {
  static auto back = WzResource::LoadTexture(
      WzResource::ui_->find(u"PartyHP.img/GaugeBar2/graduation"));
  const auto w = 200;

  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x - (float)back->w / 2),
      static_cast<float>((int)pos.y),
      static_cast<float>(back->w),
      static_cast<float>(back->h),
  };
  auto &camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= (int)camera.x;
    pos_rect.y -= (int)camera.y;
    SDL_RenderTexture(Window::renderer_, back, nullptr, &pos_rect);

    auto x = (int)pos_rect.x + 3;
    auto y = (int)pos_rect.y + 3;
    auto w = (pos_rect.w - 6);
    auto h = 4;
    SDL_SetRenderDrawColor(Window::renderer_, 255, 0, 0, 255);
    pos_rect.x = x;
    pos_rect.y = y;
    pos_rect.w = w * g.hp_percent;
    pos_rect.h = h;
    SDL_RenderFillRect(Window::renderer_, &pos_rect);

    SDL_SetRenderDrawColor(Window::renderer_, 0, 255, 0, 255);
    pos_rect.w = w * g.hp_percent_now;
    SDL_RenderFillRect(Window::renderer_, &pos_rect);
  }

  return true;
}

bool GaugeRenderSystem::RenderCharacter(SDL_FPoint pos, float hp_percent) {
  static auto back = WzResource::LoadTexture(
      WzResource::ui_->find(u"PartyHP.img/GaugeBar2/graduation"));
  const auto w = 200;

  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x - (float)back->w / 2),
      static_cast<float>((int)pos.y),
      static_cast<float>(back->w),
      static_cast<float>(back->h),
  };
  auto &camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= (int)camera.x;
    pos_rect.y -= (int)camera.y;
    SDL_RenderTexture(Window::renderer_, back, nullptr, &pos_rect);

    auto x = (int)pos_rect.x + 3;
    auto y = (int)pos_rect.y + 3;
    auto w = (pos_rect.w - 6);
    auto h = 4;
    SDL_SetRenderDrawColor(Window::renderer_, 255, 0, 0, 255);
    pos_rect.x = x;
    pos_rect.y = y;
    pos_rect.w = w * hp_percent;
    pos_rect.h = h;
    SDL_RenderFillRect(Window::renderer_, &pos_rect);
  }
  return true;
}

}  // namespace sdlms
