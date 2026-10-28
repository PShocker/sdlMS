#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_SKILL_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_SKILL_UI_SYSTEM_H_

#include <SDL3/SDL_events.h>

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

  static bool EventClickSki(SDL_Event* event);
  static bool EventClickTab(SDL_Event* event);
  static void EventVscr(SDL_Event* event);

  static void EventMotion(SDL_Event* event);
  static void EventButtonSkiUp();
  static bool EventButton(SDL_Event* event);
  static void EventClose();
  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static void EventWheel(SDL_Event* event);

  static void EventMotionBtAudio(SDL_Event* event);

  static void EventVscrStart(SDL_Event* event);
  static void EventVscrEnd();
  static void EventVscrMove(SDL_Event* event);

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
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_SKILL_UI_SYSTEM_H_
