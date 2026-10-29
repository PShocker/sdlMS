#pragma once

#include "src/client/system/ui/component/button.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <array>
#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

namespace sdlms {

class WorldmapUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderMap();
  static void RenderSpot();
  static void RenderButton();
  static std::vector<Button>& GetComponent();
  static bool RenderSpotInfo();
  static void RenderCurPos(SDL_FPoint point);

  static void EventMotion(SDL_Event* event);
  static void EventClose();

  static SDL_FPoint LoadWh();
  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static void EventClickSpot();
  static bool EventButton(SDL_Event* event);

  static inline std::optional<SDL_FPoint> drag_;

  static inline std::vector<Button> components_;

 public:
  static inline SDL_FPoint pos_;
  static inline std::u16string path_;

  static inline std::array<uint32_t, 3> pos_ani_time_;

  static bool CursorIn();

  static void Toggle();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
