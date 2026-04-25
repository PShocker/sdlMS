#include "window.h"
#include "SDL3/SDL_render.h"
#include <cstdint>
#include <cstdlib>

void window::create(const char *title, uint32_t logic_w, uint32_t logic_h,
                    uint32_t window_w, uint32_t window_h) {
  auto result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
  if (result == false) {
    std::abort();
  }
  // 设置窗口分辨率
  win = SDL_CreateWindow(title, window_w, window_h, 0);
  if (!win) {
    std::abort();
  }
  renderer = SDL_CreateRenderer(win, SDL_GPU_RENDERER);
  SDL_SetRenderVSync(renderer, 1);
  SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_NEAREST);
  // 设置缩放,实际分辨率最好是窗口分辨率的整数倍
  auto logical_mode = SDL_LOGICAL_PRESENTATION_STRETCH;
  SDL_SetRenderLogicalPresentation(renderer, logic_w, logic_h, logical_mode);
}

void window::tick() {
  dt_last = dt_now;
  dt_now = SDL_GetTicks();
  delta_time = dt_now - dt_last;
}

void window::clear() { SDL_RenderClear(renderer); }

void window::update() {
  pre_mouse_state = mouse_state;
  mouse_state = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
  SDL_RenderPresent(renderer);
}
