#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"
#include <cstdint>
class QuickSlotUiSystem {
private:
  static void RenderQuickSlot();
  static void RenderButton();

  static bool EventButton(SDL_Event *Event);
  static bool EventClickQuickslot(SDL_Event *Event);

  static SDL_FPoint LoadWh();

public:
  enum class QuickSlot {
    kHide,
    kTwo,
    kThree,
  };
  static inline QuickSlot quickSlot = QuickSlot::kTwo;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event *Event);

  static bool CursorIn();

  static void Toggle();

  static void RenderInputIcon(const GameInput &i, int x, int y,
                                uint8_t alpha = 255);
};