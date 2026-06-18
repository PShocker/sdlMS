#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/system_instance/login_notice_system_instance.h"
class login_notice_ui_system {
private:
  static SDL_FPoint load_pos();
  static void render_backgrnd();
  static void render_button();

  static void event_close();
  static bool event_button(SDL_Event *event);

public:
  static inline void (*fn)();

  static inline login_notice_system_instance::notice_type type;
  static bool render();
  static bool event(SDL_Event *event);
};