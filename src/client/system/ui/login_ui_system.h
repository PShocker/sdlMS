#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
class login_ui_system {
private:
  static SDL_FPoint load_pos();
  static void render_backgrnd();
  static void render_button();
  static void render_effect();

public:
  static bool render();
};