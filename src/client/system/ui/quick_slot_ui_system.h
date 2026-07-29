#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
class quick_slot_ui_system {
private:
  static void render_quickSlot();
  static void render_button();

  static bool event_button(SDL_Event *event);
  static bool event_click_quickslot(SDL_Event *event);

  static SDL_FPoint load_wh();

public:
  enum class quick_slot {
    hide,
    two,
    three,
  };
  static inline quick_slot quickSlot = quick_slot::two;
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};