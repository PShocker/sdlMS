#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

namespace sdlms {

class PlayerShopUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderUI();

  static bool EventUI(SDL_Event* event);

 public:
  static inline SDL_FPoint pos_;

  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
