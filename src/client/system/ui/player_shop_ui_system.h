#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_PLAYER_SHOP_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_PLAYER_SHOP_UI_SYSTEM_H_

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

namespace sdlms {

class PlayerShopUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderButton();

  static bool EventButton(SDL_Event* event);

 public:
  static inline SDL_FPoint pos_;

  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_PLAYER_SHOP_UI_SYSTEM_H_
