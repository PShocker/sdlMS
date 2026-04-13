#pragma once

#include <SDL3/SDL.h>
#include <cstdint>

class Window {
public:
  static inline SDL_Window *window;
  static inline SDL_Renderer *renderer;

  static inline float mouseX;
  static inline float mouseY;

  static inline SDL_MouseButtonFlags prevMouseState;
  static inline SDL_MouseButtonFlags mouseState;
  static const inline bool *keyboardState = SDL_GetKeyboardState(nullptr);

  static inline const char *windowTitle;

  static inline uint64_t dtNow = 0;
  static inline uint64_t dtLast = 0;
  static inline uint64_t deltaTime = 0;
  static inline bool quit;

  static void createWindow(const char *title, uint32_t width, uint32_t height);

  static void tick();

  static void clear();
  static void update();
};
