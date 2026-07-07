#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>
class beauty_ui_system {
private:
  static void render_backgrnd();
  static void render_npc();
  static void render_button();
  static void render_text();

  static SDL_FPoint load_wh();

  static void event_button_close();
  static void event_button_ok();

  static void event_button_prev();
  static void event_button_next();

  static bool event_button(SDL_Event *event);

public:
  static bool render();

  static void open();
  static void close();

  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();

  static inline SDL_FPoint pos;
};