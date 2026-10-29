#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>

#include <cstdint>
#include <flat_map>

namespace sdlms {

class CraftUiSystem : public UiComponentHost<CraftUiSystem> {
 private:
  static void RenderBackgrnd();

  static bool EventUI(SDL_Event* event);
  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static bool Run();

  static inline std::optional<SDL_FPoint> drag_;


 public:
  static UIComponents& GetComponent();
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
