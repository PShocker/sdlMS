#pragma once

#include "SDL3/SDL_events.h"
class character_choose_ui_system {
private:
  static void render_button();

public:
  static bool render();
  static bool event(SDL_Event *event);
};