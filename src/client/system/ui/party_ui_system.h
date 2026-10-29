#pragma once

#include <SDL3/SDL_events.h>

#include <optional>

namespace sdlms {

class PartyUiSystem {
 private:
  static void RenderBackgrnd(float x, float y, float w, float h);
  static void RenderPartyHp();

  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static inline std::optional<SDL_FPoint> drag_;

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
