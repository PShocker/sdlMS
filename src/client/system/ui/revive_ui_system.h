#pragma once

#include "SDL3/SDL_events.h"
#include <optional>
namespace sdlms {

class ReviveUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderButton();

  static bool EventButton(SDL_Event *Event);
  static void EventButtonOk();

  static void EventMotionBtAudio(SDL_Event *Event);

  static SDL_FPoint LoadWh();

public:
  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event *Event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
