#pragma once

#include "SDL3/SDL_events.h"
#include <string>
class cursor_logic_system {
private:
  static bool event_left_click();

  static bool run_package_motion();
  static bool run_animate();

  static void run_cursor_ui();
  static void run_cursor_action(const std::u16string &action);

  static bool run_default();

  static bool event_cursor_hand(SDL_Event *event);
  static bool event_character_info(SDL_Event *event);
  static bool event_npc(SDL_Event *event);

public:
  static bool run();

  static bool event(SDL_Event *event);
};