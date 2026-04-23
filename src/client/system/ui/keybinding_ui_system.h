#pragma once

#include "SDL3/SDL_events.h"
class keybinding_ui_system {
public:
  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};