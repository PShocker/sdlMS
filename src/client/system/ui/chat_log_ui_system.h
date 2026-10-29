#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>

namespace sdlms {

class ChatLogUiSystem : public UiComponentHost<ChatLogUiSystem> {
 private:
  static void RenderBackgrnd();
  static void RenderChats();

  static bool EventUI(SDL_Event* event);


 public:
  static UIComponents& GetComponent();
  static inline int page_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
