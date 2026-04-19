#pragma once

#include "SDL3/SDL_events.h"
class keyboard_input_system {
public:
  static bool event(SDL_Event *event);
};