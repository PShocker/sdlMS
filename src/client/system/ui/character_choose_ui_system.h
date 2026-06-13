#pragma once

#include "SDL3/SDL_events.h"
class character_choose_ui_system {
private:
  static SDL_FPoint load_pos();
  static void render_backgrnd();
  static void render_button();
  static void render_effect();
  static void render_banner();

public:
  static bool render();
  static bool event(SDL_Event *event);
};