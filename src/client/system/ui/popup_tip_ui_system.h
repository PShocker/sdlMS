#pragma once

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

  static bool EventUI(SDL_Event* event);

  static void Destroy();

 public:
  static inline SDL_FPoint pos_ = {429, -32};

  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
