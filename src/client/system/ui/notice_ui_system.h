#pragma once

#include "SDL3/SDL_events.h"
#include <cstdint>
class notice_ui_system {
private:
  static void render_backgrnd();
  static void render_button();
  static void render_text();

  static void event_close();

  static bool event_button(SDL_Event *event);

  static SDL_FPoint load_wh();

public:
  enum class notice_enum {
    shopbuy,
    shopbuy_mul,
    shopbuy_no_meso,
    shopbuy_no_space,

    equip_no_ability,
    equip_no_space,
  };

  static inline notice_enum type;
  static inline SDL_FPoint pos;

  static bool render();

  static void open();
  static void close();

  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};