#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>

#include <optional>
#include <vector>

namespace sdlms {

class ReviveUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderUI();
  static UIComponents& GetComponent();

  static bool EventUI(SDL_Event* event);
  static void EventButtonOk();

  static void EventMotionBtAudio(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static inline UIComponents components_;

 public:
  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
