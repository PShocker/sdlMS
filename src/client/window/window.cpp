#include "Window.h"
#include <cstdlib>

void Window::createWindow(const char *title, unsigned int width,
                          unsigned int height) {
  int result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
  if (result < 0) {
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
