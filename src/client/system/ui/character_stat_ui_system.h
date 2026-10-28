#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <optional>
#include <vector>
class CharacterStatUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderText();
  static void RenderRemainAp();
  static void RenderButton();
  static void RenderDetail();
  static SDL_FPoint LoadWh();

  static void EventButtonHpInc();
  static void EventButtonHpIncMax();
  static void EventButtonMpInc();
  static void EventButtonMpIncMax();
  static void EventButtonStrInc();
  static void EventButtonStrIncMax();
  static void EventButtonDexInc();
  static void EventButtonDexIncMax();
  static void EventButtonIntInc();
  static void EventButtonIntIncMax();
  static void EventButtonLukInc();
  static void EventButtonLukIncMax();

  static void EventButtonDetailShow();
  static void EventButtonDetailHide();
  static void EventTop();
  static void EventDragStart(SDL_Event *Event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event *Event);

  static bool EventButton(SDL_Event *Event);

  static inline std::optional<SDL_FPoint> drag;

  static inline bool detail;

public:
  static inline SDL_FPoint pos;
  static bool Render();

  static bool CursorIn();

  static void Open();
  static void Close();
  static bool Event(SDL_Event *Event);

  static void Toggle();
};