#pragma once

#include "SDL3/SDL_events.h"
#include <cstdint>
#include <flat_map>

class package_ui_system {
private:
  static void render_backgrnd();
  static void render_tab();

  static void event_button(SDL_Event *event);

  static SDL_FPoint load_wh();
public:
  static inline SDL_FPoint pos;

  static inline uint8_t active_tab;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};