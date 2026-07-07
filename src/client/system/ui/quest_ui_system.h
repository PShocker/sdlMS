#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <optional>
#include <string>
class quest_ui_system {
private:
  static void render_backgrnd();
  static void render_button();
  static void render_quests();
  static void render_quest_detail();

  static void event_button(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static SDL_FPoint load_wh();

  static inline std::optional<SDL_FPoint> drag;

public:
  static inline SDL_FPoint pos;

  static inline std::u16string detail;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};