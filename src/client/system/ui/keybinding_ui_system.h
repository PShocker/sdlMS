#pragma once

#include "src/client/game/game_input.h"

#include <SDL3/SDL_events.h>

#include <cstdint>
#include <optional>

namespace sdlms {

class KeybindingUiSystem {
 private:
  static void RenderButton();
  static bool RenderInfo();

  static void RenderBackgrnd();
  static void RenderBackgrnd2();
  static void RenderBackgrndKey();
  static void RenderTab();
  static void RenderRemainKey();
  static SDL_FPoint LoadWh();
  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static void EventMotionBtAudio(SDL_Event* event);

  static void EventMotion(SDL_Event* event);

  static void EventButtonClose();
  static void EventButtonDefault();
  static void EventButtonDelete();
  static void EventButtonQuickSlot();
  static void EventButtonPrePage1();
  static void EventButtonPrePage2();
  static void EventButtonPrePage3();
  static void EventButtonSave();
  static void EventButtonUndo();

  static void EventBackgrndKey();
  static void EventRemainKey();

  static bool EventButton(SDL_Event* event);

  static inline std::optional<SDL_FPoint> drag_;
  static inline uint8_t active_tab_;

 public:
  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static void RenderRemainIcon(int i, int x, int y, uint8_t alpha = 255);
  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
