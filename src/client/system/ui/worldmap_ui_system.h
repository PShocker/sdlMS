#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <array>
#include <cstdint>
#include <flat_map>
#include <string>

class WorldmapUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderMap();
  static void RenderSpot();
  static void RenderButton();
  static bool RenderSpotInfo();
  static void RenderCurPos(SDL_FPoint point);

  static void EventMotion(SDL_Event *Event);
  static void EventClose();

  static SDL_FPoint LoadWh();
  static void EventTop();
  static void EventDragStart(SDL_Event *Event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event *Event);

  static void EventClickSpot();
  static bool EventButton(SDL_Event *Event);

  static inline std::optional<SDL_FPoint> drag;

public:
  static inline SDL_FPoint pos;
  static inline std::u16string path;

  static inline std::array<uint32_t, 3> pos_ani_time;

  static bool CursorIn();

  static void Toggle();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event *Event);
};