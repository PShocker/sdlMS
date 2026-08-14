#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_popup_tip.h"
class popup_tip_ui_system {
private:
  static void render_backgrnd(game_popup_tip &t);
  static void render_text(game_popup_tip &t);
  static void render_button(game_popup_tip &t);

  static void event_button_party(game_popup_tip &t);
  static void event_button_trade(game_popup_tip &t);
  static void event_button_ok(game_popup_tip &t);
  static void event_button_cancel(game_popup_tip &t);

  static bool event_button(SDL_Event *event);

public:
  static inline SDL_FPoint pos;

  static bool render();
  static bool event(SDL_Event *event);
};