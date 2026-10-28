#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
class PlayerShopUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderButton();

  static bool EventButton(SDL_Event *Event);

public:
  static inline SDL_FPoint pos;

  static bool Render();
  static bool Event(SDL_Event *Event);
};