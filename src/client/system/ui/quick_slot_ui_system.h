#pragma once

#include "src/client/game/game_input.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_scancode.h>

#include <cstdint>

namespace sdlms {

class QuickSlotUiSystem {
 private:
  static void RenderQuickSlot();
  static void RenderButton();

  static bool EventButton(SDL_Event* event);
  static bool EventClickQuickslot(SDL_Event* event);

  static SDL_FPoint LoadWh();

 public:
  enum class QuickSlot {
    kHide,
    kTwo,
    kThree,
  };
  static inline QuickSlot quick_slot_ = QuickSlot::kTwo;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();

  static void RenderInputIcon(const GameInput& i, int x, int y,
                              uint8_t alpha = 255);
};

}  // namespace sdlms
