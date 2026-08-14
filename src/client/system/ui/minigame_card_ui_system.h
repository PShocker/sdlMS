#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
class minigame_card_ui_system {
private:
  static void render_backgrnd();
  static SDL_FPoint load_wh();

public:
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};