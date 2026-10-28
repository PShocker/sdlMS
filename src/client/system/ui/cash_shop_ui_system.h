#pragma once

#include "SDL3/SDL_events.h"
class CashShopUiSystem {
private:
  static void render_backgrnd();

public:
  static bool render();
  static bool event(SDL_Event *event);
};