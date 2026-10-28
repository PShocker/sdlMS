#pragma once

#include "SDL3/SDL_events.h"
namespace sdlms {

class KeyboardInputSystem {
public:
  static bool Event(SDL_Event *Event);
  static void Reset();
};

}  // namespace sdlms
