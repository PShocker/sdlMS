#pragma once

#include "src/client/game/game_input.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_scancode.h>

#include <cstdint>
#include <optional>

namespace sdlms {

class QuickSlotUiSystem {
 private:
  static void RenderQuickSlot();

  static bool RenderInfo();
  static void EventMotion(SDL_Event* event);

  static bool EventClickQuickslot(SDL_Event* event);

  static SDL_FPoint LoadWh();

  // 鼠标悬浮的快捷键
  static inline std::optional<SDL_Scancode> mouse_key_;

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
