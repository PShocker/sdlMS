#pragma once

#include "src/client/component/button.h"

#include <SDL3/SDL_events.h>

#include <optional>
#include <vector>

namespace sdlms {

class ReviveUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderButton();
  static std::vector<Button>& GetComponent();

  static bool EventButton(SDL_Event* event);
  static void EventButtonOk();

  static void EventMotionBtAudio(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static inline std::vector<Button> components_;

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
