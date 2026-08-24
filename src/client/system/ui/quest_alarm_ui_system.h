#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <optional>
class quest_alarm_ui_system {
private:
  static void render_backgrnd();
  static void render_quests();

  static SDL_FPoint load_wh();

  static inline std::optional<SDL_FPoint> drag;

public:
  static inline bool max;

  static inline SDL_FPoint pos;

  static bool render();
  static bool event(SDL_Event *event);
};