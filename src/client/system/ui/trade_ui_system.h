#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_TRADE_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_TRADE_UI_SYSTEM_H_

#include "src/client/game/game_character.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

namespace sdlms {

class TradeUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderButton();

  static void EventButtonClose();
  static bool EventButton(SDL_Event* event);

 public:
  static inline SDL_FPoint pos_;
  static inline GameCharacter other_;

  static bool CursorIn();

  static void Toggle();
  static SDL_FPoint LoadWh();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_TRADE_UI_SYSTEM_H_
