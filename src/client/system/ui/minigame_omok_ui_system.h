#pragma once

#include "SDL3/SDL_events.h"
namespace sdlms {

class MinigameOmokUiSystem {
private:
  static void RenderBackgrnd();
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
