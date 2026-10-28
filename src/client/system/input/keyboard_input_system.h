#pragma once

#include "SDL3/SDL_events.h"
class KeyboardInputSystem {
public:
  static bool Event(SDL_Event *Event);
  static void Reset();
};