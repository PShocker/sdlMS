#pragma once

#include "SDL3/SDL_events.h"
class popup_tip_ui_system {
private:
  static void render_backgrnd(int i);
  static void render_button(int i);

  static void event_button_party(int i);
  static void event_button_trade(int i);
  static void event_button_ok(int i);
  static void event_button_cancel();

  static bool event_button(SDL_Event *event);

public:
  static bool render();
  static bool event(SDL_Event *event);
};