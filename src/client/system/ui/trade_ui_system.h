#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <optional>
class trade_ui_system {
private:
  static void render_backgrnd();

  static void event_button_close();
  static bool event_button(SDL_Event *event);

  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);
  static inline std::optional<SDL_FPoint> drag;

public:
  static inline SDL_FPoint pos;
  static inline uint64_t other_id;

  static bool cursor_in();

  static void toggle();
  static SDL_FPoint load_wh();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};