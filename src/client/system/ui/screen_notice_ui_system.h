#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <string>
class screen_notice_ui_system {
private:
  static void render_backgrnd();

public:
  static inline SDL_FPoint pos;
  static inline std::u16string path;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();

  static SDL_FPoint load_wh();
};