#include "fade_system_instance.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"

bool fade_system_instance::render() {
  SDL_FRect pos_rect{
      0,
      0,
      (camera_game_instance::camera.w),
      (camera_game_instance::camera.h),
  };
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
  SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, mask_alpha);
  SDL_RenderFillRect(window::renderer, &pos_rect);
  return true;
}

void fade_system_instance::enter(std::function<void()> cb) {
  // save texture
  SDL_Rect pos_rect{
      0,
      0,
      static_cast<int>(camera_game_instance::camera.w),
      static_cast<int>(camera_game_instance::camera.h),
  };
  auto surface = SDL_RenderReadPixels(window::renderer, &pos_rect);
  if (backgrnd) {
    SDL_DestroyTexture(backgrnd);
    backgrnd = nullptr;
  }
  backgrnd = SDL_CreateTextureFromSurface(window::renderer, surface);
  SDL_DestroySurface(surface);

  system::logic_systems = {run};
  system::render_systems = {render};
  system::event_systems = {};

  mask_alpha = 0;
  fn = cb;
}

bool fade_system_instance::run() {
  if (mask_alpha <= 255) {
    mask_alpha += 10;
  } else {
    fn();
    return false;
  }
  return true;
}