#ifndef SDLMS_SRC_CLIENT_SYSTEM_INPUT_KEYBOARD_INPUT_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_INPUT_KEYBOARD_INPUT_SYSTEM_H_

#include <SDL3/SDL_events.h>
namespace sdlms {

class KeyboardInputSystem {
 public:
  static bool Event(SDL_Event* Event);
  static void Reset();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_INPUT_KEYBOARD_INPUT_SYSTEM_H_
