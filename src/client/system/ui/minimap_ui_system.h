#pragma once

#include "src/client/system/ui/component/button.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

class MinimapUiSystem {
 private:
  static void RenderMaxBackgrnd();
  static void RenderMinBackgrnd();
  static void RenderButton();
  static std::vector<Button>& GetComponent();
  static void RenderNameText();
  static void RenderStreetText();
  static void RenderCanvasLife();
  static void RenderPortal();
  static void RenderMark();
  static void RenderCanvas();
  static void RenderMini();
  static void RenderName();

  static void EventWorldMap();
  static void EventMin();
  static void EventMax();

  static bool EventButton(SDL_Event* event);
  static void EventTop();

  static void EventMotionBtAudio(SDL_Event* event);

  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static SDL_FPoint LoadCanvasPoint(SDL_FPoint point, int8_t ox, int8_t oy);
  static SDL_FPoint LoadCanvasWh();
  static SDL_FPoint LoadCanvasO();
  static SDL_FPoint LoadCanvasLt();
  static SDL_FRect LoadCanvasViewport();
  static SDL_Texture* LoadCanvasTexture();

  static SDL_FPoint LoadWh();

  static inline std::optional<SDL_FPoint> drag_;

  static inline std::vector<Button> components_;

 public:
  static inline SDL_FPoint pos_ = {0, 0};
  static inline bool max_ = true;
  static inline bool disable_;

  struct MapName {
    std::u16string street_name;
    std::u16string map_name;
    std::u16string map_desc = u"";
  };
  static MapName LoadMapName(uint32_t map_id);

  static void Load();
  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();
};

}  // namespace sdlms
