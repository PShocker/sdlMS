#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_npc.h"
#include <optional>
#include <string>
class CursorLogicSystem {
private:
  static std::optional<GameNpc> cursor_npc();

  static bool run_package_motion();
  static bool run_vscroll_motion();
  static bool run_animate();

  static void run_cursor_ui();
  static void run_cursor_action(const std::u16string &action);

  static bool run_default();

  static bool event_cursor_hand(SDL_Event *event);
  static bool event_character(SDL_Event *event);
  static bool event_npc(SDL_Event *event);

public:
  static bool run();

  static bool event_click_npc(GameNpc npc);
  static bool event(SDL_Event *event);
};