#include "Window.h"
#include <cstdint>
#include <cstdlib>

void Window::createWindow(const char *title, uint32_t width, uint32_t height) {
  auto result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
  if (result == false) {
    std::abort();
  }
  window = SDL_CreateWindow(title, width, height, 0);
  if (!window) {
    std::abort();
  }
  renderer = SDL_CreateRenderer(window, NULL);
  SDL_SetRenderVSync(renderer, 1);
  windowTitle = title;
}

void Window::tick() {
  dtLast = dtNow;
  dtNow = SDL_GetTicks();
  deltaTime = dtNow - dtLast;
}

void Window::clear() { SDL_RenderClear(renderer); }

void Window::update() {
  prevMouseState = mouseState;
  mouseState = SDL_GetMouseState(&mouseX, &mouseY);
  SDL_RenderPresent(renderer);
}
