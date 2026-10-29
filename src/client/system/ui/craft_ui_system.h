#pragma once

#include <SDL3/SDL_events.h>

#include <cstdint>
#include <flat_map>

namespace sdlms {

class CraftUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderTab();

  static bool EventButton(SDL_Event* event);
  static void EventTab(SDL_Event* event);
  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static bool Run();

  static inline std::optional<SDL_FPoint> drag_;

 public:
  static inline SDL_FPoint pos_;

  static inline uint8_t selected_tab_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
