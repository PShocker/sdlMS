#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
class login_ui_system {
private:
  static bool login_animate();

  static SDL_FPoint load_pos();
  static void render_backgrnd();
  static void render_button();
  static void render_effect();

  static void event_button_login();
  static void event_button_login_save();
  static void event_button_find_id();
  static void event_button_find_pw();
  static void event_button_register();
  static void event_button_homepage();
  static void event_button_quit();

  static bool event_button(SDL_Event *event);
public:
  static bool render();
  static bool event(SDL_Event *event);
};