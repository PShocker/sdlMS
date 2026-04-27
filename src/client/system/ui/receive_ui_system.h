#pragma once

#include "SDL3/SDL_events.h"
#include <optional>
class receive_ui_system {
private:
  static void render_backgrnd();
  static void render_button();

  static bool event_button(SDL_Event *event);
  static void event_button_ok();

  static void event_tab(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static SDL_FPoint load_wh();

  static inline std::optional<SDL_FPoint> drag;

public:
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};