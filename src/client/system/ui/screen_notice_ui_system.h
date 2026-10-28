#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <string>
class ScreenNoticeUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderText();

public:
  static inline SDL_FPoint pos;
  static inline std::u16string path;
  static inline std::u16string text;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event *Event);

  static bool CursorIn();

  static void Toggle();

  static SDL_FPoint LoadWh();
};