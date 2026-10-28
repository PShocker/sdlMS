#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
class TradeUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderButton();

  static void EventButtonClose();
  static bool EventButton(SDL_Event *Event);

public:
  static inline SDL_FPoint pos;
  static inline GameCharacter other;

  static bool CursorIn();

  static void Toggle();
  static SDL_FPoint LoadWh();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event *Event);
};