#pragma once

#include <SDL3/SDL_events.h>

namespace sdlms {

class CashShopUiSystem {
 private:
  static void RenderBackgrnd();

 public:
  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
