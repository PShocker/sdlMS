#pragma once

#include "SDL3/SDL_events.h"
class KeyboardInputSystem {
public:
  static bool event(SDL_Event *event);
  static void reset();
};