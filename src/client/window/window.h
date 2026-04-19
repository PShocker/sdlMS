#pragma once

#include "SDL3/SDL_rect.h"
#include <SDL3/SDL.h>
#include <cstdint>

class window {
public:
  static inline SDL_Window *win;
  static inline SDL_Renderer *renderer;

  static inline SDL_FPoint mouse_pos;

  static inline SDL_MouseButtonFlags pre_mouse_state;
  static inline SDL_MouseButtonFlags mouse_state;
  static const inline bool *keyboard_State = SDL_GetKeyboardState(nullptr);

  static inline uint64_t dt_now = 0;
  static inline uint64_t dt_last = 0;
  static inline uint64_t delta_time = 0;

  static void create(const char *title, uint32_t width, uint32_t height);

  static void tick();

  static void clear();
  static void update();
};
