#pragma once

#include "SDL3/SDL_events.h"
#include <cstdint>
#include <flat_map>

class skill_ui_system {
public:
  static void render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
};