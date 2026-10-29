#pragma once

#include <SDL3/SDL_events.h>

namespace sdlms {

class ChatLogUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderChats();
  static void RenderVscr();

  static bool EventUI(SDL_Event* event);

 public:
  static inline int page_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
