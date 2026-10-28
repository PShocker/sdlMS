#pragma once

#include "SDL3/SDL_events.h"
#include <cstdint>
#include <optional>
namespace sdlms {


class SkillUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderTab();
  static void RenderSkillEntry();
  static void RenderScroll();
  static bool RenderInfo();
  static void RenderButton();
  static void RenderBook();
  static void RenderPoint();

  static SDL_FPoint LoadWh();

  static bool EventClickSki(SDL_Event *Event);
  static bool EventClickTab(SDL_Event *Event);
  static void EventVscr(SDL_Event *Event);

  static void EventMotion(SDL_Event *Event);
  static void EventButtonSkiUp();
  static bool EventButton(SDL_Event *Event);
  static void EventClose();
  static void EventTop();
  static void EventDragStart(SDL_Event *Event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event *Event);

  static void EventWheel(SDL_Event *Event);

  static void EventMotionBtAudio(SDL_Event *Event);
  
  static void EventVscrStart(SDL_Event *Event);
  static void EventVscrEnd();
  static void EventVscrMove(SDL_Event *Event);

  static inline bool vscr_motion_;

  static inline std::optional<SDL_FPoint> drag_;

  static inline uint8_t active_tab_;
  static inline uint8_t page_;

public:
  static inline SDL_FPoint pos_;

  static bool CursorIn();

  static void Toggle();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event *Event);
};

}  // namespace sdlms
