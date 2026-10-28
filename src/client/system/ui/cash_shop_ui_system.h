#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_CASH_SHOP_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_CASH_SHOP_UI_SYSTEM_H_

#include <SDL3/SDL_events.h>

namespace sdlms {

class CashShopUiSystem {
 private:
  static void RenderBackgrnd();

 public:
  static bool Render();
  static bool Event(SDL_Event* Event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_CASH_SHOP_UI_SYSTEM_H_
