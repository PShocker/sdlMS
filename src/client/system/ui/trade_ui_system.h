#pragma once

#include "src/client/component/button.h"
#include "src/client/game/game_character.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <vector>

namespace sdlms {

class TradeUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderButton();
  static std::vector<Button>& GetComponent();

  static void EventButtonClose();
  static bool EventButton(SDL_Event* event);

  static inline std::vector<Button> components_;

 public:
  static inline SDL_FPoint pos_;
  static inline GameCharacter other_;

  static bool CursorIn();

  static void Toggle();
  static SDL_FPoint LoadWh();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
