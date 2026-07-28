#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
class trade_ui_system {
private:
  static void render_backgrnd();
  static void render_button();

  static void event_button_close();
  static bool event_button(SDL_Event *event);

public:
  static inline SDL_FPoint pos;
  static inline game_character other;

  static bool cursor_in();

  static void toggle();
  static SDL_FPoint load_wh();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};