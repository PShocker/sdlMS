#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <optional>
class storage_ui_system {
private:
  static std::optional<uint32_t> load_mouse_index();
  static SDL_FPoint load_wh();
  
  static void render_backgrnd();
  static void render_items();
  static void render_item_info();

  static bool event_button(SDL_Event *event);
  static void event_close();

public:
  static inline SDL_FPoint pos;

  static bool cursor_in();

  static void toggle();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};