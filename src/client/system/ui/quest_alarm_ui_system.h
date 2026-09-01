#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <flat_set>
#include <optional>
#include <string>
class quest_alarm_ui_system {
private:
  static void render_backgrnd();
  static void render_quests();
  static void render_button();
  static void render_title();

  static SDL_FPoint load_wh();

  static void event_button_auto();
  static void event_button_max();
  static void event_button_min();
  static void event_button_close();

  static void event_quest_close();

  static bool event_button(SDL_Event *event);

  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static inline std::optional<SDL_FPoint> drag;

public:
  static inline std::flat_set<std::u16string> quests;

  static inline bool max = true;

  static inline SDL_FPoint pos;

  static void load_quests();

  static void open();
  static void close();
  
  static bool cursor_in();

  static bool render();
  static bool event(SDL_Event *event);
};