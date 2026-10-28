#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_input.h"
#include <cstdint>
#include <optional>
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
  static void EventDragStart(SDL_Event *Event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event *Event);

  static void EventMotionBtAudio(SDL_Event *Event);

  static void EventMotion(SDL_Event *Event);
  
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


  static bool EventButton(SDL_Event *Event);

  static inline std::optional<SDL_FPoint> drag_;
  static inline uint8_t active_tab_;

public:
  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static void RenderRemainIcon(int i, int x, int y, uint8_t alpha = 255);
  static bool Render();
  static bool Event(SDL_Event *Event);

  static bool CursorIn();

  static void Toggle();
};