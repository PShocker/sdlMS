#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_popup_tip.h"
class PopupTipUiSystem {
private:
  static void render_backgrnd(GamePopupTip &t);
  static void render_text(GamePopupTip &t);
  static void render_button(GamePopupTip &t);

  static void event_button_party(GamePopupTip &t);
  static void event_button_trade(GamePopupTip &t);
  static void event_button_ok(GamePopupTip &t);
  static void event_button_cancel(GamePopupTip &t);

  static bool event_button(SDL_Event *event);

  static void destory();

public:
  static inline SDL_FPoint pos = {429, -32};

  static bool render();
  static bool event(SDL_Event *event);
};