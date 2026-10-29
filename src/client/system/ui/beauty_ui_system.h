#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

class BeautyUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderNpc();
  static void RenderUI();
  static void RenderText();

  static SDL_FPoint LoadWh();

  static void EventButtonClose();
  static void EventButtonOk();

  static void EventButtonPrev();
  static void EventButtonNext();

  static bool EventButton(SDL_Event* event);

 public:
  static bool Render();

  static void Open();
  static void Close();

  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();

  static inline SDL_FPoint pos_;
};

}  // namespace sdlms
