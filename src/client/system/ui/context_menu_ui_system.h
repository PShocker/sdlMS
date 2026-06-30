
#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
class context_menu_ui_system {
private:
  static void render_backgrnd();

public:
  static inline SDL_FPoint pos;
  static bool render();

  static bool cursor_in();

  static void open();
  static void close();
  static bool event(SDL_Event *event);

  static void toggle();
};