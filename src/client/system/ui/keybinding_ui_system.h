#pragma once

#include "SDL3/SDL_events.h"
#include <optional>
class keybinding_ui_system {
private:
  static void render_backgrnd();
  static void render_backgrnd_key();
  static SDL_FPoint load_wh();
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

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