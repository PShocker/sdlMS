#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"

class minimap_ui_system {
public:
  static inline SDL_FPoint pos;
  static inline bool max;
  static inline bool disable;

  static void load();
  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

private:
  static void render_backgrnd();
  static void render_button();
  static void render_name_text();
  static void render_street_text();
  static void render_canvas_life();
  static void render_portal();
  static void render_mark();
  static void render_canvas();
  static void render_mini();

  static bool event_button(SDL_Event *event);
};