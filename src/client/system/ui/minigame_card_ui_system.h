#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
class MinigameCardUiSystem {
private:
  static void RenderBackgrnd();
  static SDL_FPoint LoadWh();

public:
  static inline SDL_FPoint pos;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event *Event);

  static bool CursorIn();

  static void Toggle();
};