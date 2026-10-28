#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_POPUP_TIP_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_POPUP_TIP_UI_SYSTEM_H_

#include "src/client/game/game_popup_tip.h"

#include <SDL3/SDL_events.h>
namespace sdlms {

class PopupTipUiSystem {
 private:
  static void RenderBackgrnd(GamePopupTip& t);
  static void RenderText(GamePopupTip& t);
  static void RenderButton(GamePopupTip& t);

  static void EventButtonParty(GamePopupTip& t);
  static void EventButtonTrade(GamePopupTip& t);
  static void EventButtonOk(GamePopupTip& t);
  static void EventButtonCancel(GamePopupTip& t);

  static bool EventButton(SDL_Event* Event);

  static void Destory();

 public:
  static inline SDL_FPoint pos = {429, -32};

  static bool Render();
  static bool Event(SDL_Event* Event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_POPUP_TIP_UI_SYSTEM_H_
