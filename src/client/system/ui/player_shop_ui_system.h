#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
class player_shop_ui_system {
private:
  static void render_backgrnd();
  static void render_button();

  static bool event_button(SDL_Event *event);

public:
  static inline SDL_FPoint pos;

  static bool render();
  static bool event(SDL_Event *event);
};