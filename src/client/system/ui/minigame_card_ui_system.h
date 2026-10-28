#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_MINIGAME_CARD_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_MINIGAME_CARD_UI_SYSTEM_H_

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

namespace sdlms {

class MinigameCardUiSystem {
 private:
  static void RenderBackgrnd();
  static SDL_FPoint LoadWh();

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

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_MINIGAME_CARD_UI_SYSTEM_H_
