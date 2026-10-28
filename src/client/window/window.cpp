#include "window.h"
#include "SDL3/SDL_render.h"
#include "src/server/server_main.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
namespace sdlms {


void Window::Create(const char *title, uint32_t logic_w, uint32_t logic_h,
                    uint32_t window_w, uint32_t window_h) {
  // hint
  SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "composition");
  SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60");

  auto result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
  if (result == false) {
    std::abort();
  }
  // 设置窗口分辨率
  win_ = SDL_CreateWindow(title, window_w, window_h, 0);
  if (!win_) {
    std::abort();
  }
  renderer_ = SDL_CreateRenderer(win_, SDL_GPU_RENDERER);
  SDL_SetDefaultTextureScaleMode(renderer_, SDL_SCALEMODE_NEAREST);
  // 设置缩放,实际分辨率最好是窗口分辨率的整数倍
  auto logical_mode = SDL_LOGICAL_PRESENTATION_STRETCH;
  SDL_SetRenderLogicalPresentation(renderer_, logic_w, logic_h, logical_mode);
}

void Window::Tick() {
  dt_last_ = dt_now_;
  dt_now_ = SDL_GetTicks();
  delta_time_ = dt_now_ - dt_last_;
  if (ServerMain::host_) {
    dt_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::system_clock::now().time_since_epoch())
                  .count();
  } else {
    dt_time_ = dt_time_base_ + (dt_now_ - dt_time_end_);
  }
}

void Window::Clear() { SDL_RenderClear(renderer_); }

void Window::Update() {
  pre_mouse_state_ = mouse_state_;
  mouse_state_ = SDL_GetMouseState(&mouse_pos_.x, &mouse_pos_.y);
  SDL_RenderPresent(renderer_);
}

}  // namespace sdlms
