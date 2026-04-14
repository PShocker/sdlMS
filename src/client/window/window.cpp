#include "Window.h"
#include <cstdint>
#include <cstdlib>

void window::createWindow(const char *title, uint32_t width, uint32_t height) {
  auto result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
  if (result == false) {
    std::abort();
  }
  win = SDL_CreateWindow(title, width, height, 0);
  if (!win) {
    std::abort();
  }
  renderer = SDL_CreateRenderer(win, NULL);
  SDL_SetRenderVSync(renderer, 1);
}

void window::tick() {
  dt_last = dt_now;
  dt_now = SDL_GetTicks();
  delta_time = dt_now - dt_last;
}

void window::clear() { SDL_RenderClear(renderer); }

void window::update() {
  pre_mouse_state = mouse_state;
  mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
  SDL_RenderPresent(renderer);
}
