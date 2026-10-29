#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

namespace sdlms {

class PlayerShopUiSystem {
 private:


 public:
  static inline SDL_FPoint pos_;

  static bool Render();
};

}  // namespace sdlms
