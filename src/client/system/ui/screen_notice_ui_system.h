#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <string>

namespace sdlms {

class ScreenNoticeUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderText();

 public:
  static inline SDL_FPoint pos_;
  static inline std::u16string path_;
  static inline std::u16string text_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();

  static SDL_FPoint LoadWh();
};

}  // namespace sdlms
