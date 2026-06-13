#pragma once

#include "SDL3/SDL_events.h"
class character_choose_ui_system {
private:
  static bool back_animate();

  static SDL_FPoint load_pos();
  static void render_backgrnd();
  static void render_button();
  static void render_effect();
  static void render_banner();

  static void event_button_select();
  static void event_button_new();
  static void event_button_delete();
  static void event_button_back();

  static bool event_button(SDL_Event *event);

public:
  static bool render();
  static bool event(SDL_Event *event);
};