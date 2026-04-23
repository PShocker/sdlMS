#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/ui/ui_base.h"
#include <cstdint>
#include <flat_map>

class statusbar_ui_system {
public:
  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
private:
  static void render_backgrnd();
  static void render_button();
};