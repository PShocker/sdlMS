#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>

#include <optional>
#include <vector>

namespace sdlms {

class ReviveUiSystem : public UiComponentHost<ReviveUiSystem> {
 private:
  static void RenderBackgrnd();
  static void RenderText();

  static bool EventUI(SDL_Event* event);
  static void EventButtonOk();

  static void EventMotionBtAudio(SDL_Event* event);

  static SDL_FPoint LoadWh();


 public:
  static UIComponents& GetComponent();
  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
