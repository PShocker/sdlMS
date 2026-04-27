#pragma once

#include "SDL3/SDL_events.h"
#include <cstdint>
#include <flat_map>

class package_ui_system {
private:
  static void render_backgrnd();
  static void render_tab();
  static void render_items();

  static void event_button(SDL_Event *event);
  static void event_tab(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static SDL_FPoint load_wh();

  static bool run();

  static inline std::optional<SDL_FPoint> drag;

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