#pragma once

#include "SDL3/SDL_events.h"
#include <optional>
class revive_ui_system {
private:
  static void render_backgrnd();
  static void render_button();

  static bool event_button(SDL_Event *event);
  static void event_button_ok();

  static SDL_FPoint load_wh();
public:
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};