#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include <cstdint>
#include <optional>

class minimap_ui_system {

private:
  static void render_max_backgrnd();
  static void render_min_backgrnd();
  static void render_button();
  static void render_name_text();
  static void render_street_text();
  static void render_canvas_life();
  static void render_portal();
  static void render_mark();
  static void render_canvas();
  static void render_mini();

  static bool event_button(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static SDL_FPoint load_canvas_point(SDL_FPoint point, int8_t ox, int8_t oy);
  static SDL_FPoint load_canvas_wh();
  static SDL_FPoint load_canvas_o();
  static SDL_FPoint load_canvas_lt();
  static SDL_FRect load_canvas_viewport();
  static SDL_Texture *load_canvas_texture();

  static SDL_FPoint load_wh();

  static inline std::optional<SDL_FPoint> drag;

public:
  static inline SDL_FPoint pos = {5, 5};
  static inline bool max = true;
  static inline bool disable;

  static void load();
  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
};