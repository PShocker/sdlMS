#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <optional>
#include <vector>
class character_info_ui_system {
private:
  static void render_backgrnd();

  static SDL_FPoint load_wh();
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static bool run();

  static inline std::optional<SDL_FPoint> drag;

  static inline bool item_list;
  static inline bool pet;

public:
  static inline SDL_FPoint pos;

  static bool cursor_in();

  static void toggle();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};