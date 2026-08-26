#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_quest.h"
#include <array>
#include <flat_set>
#include <optional>
#include <string>
class quest_ui_system {
private:
  static int load_vscr_num0();
  static int load_vscr_num1();

  static void render_backgrnd();
  static void render_button();
  static void render_quests();
  static void render_quest_detail();
  static void render_tab();
  static void render_area_name(int i, int y);
  static void render_quest(game_quest &q, int y);
  static void render_vscr();

  static void event_fold();

  static bool event_vscr(SDL_Event *event);
  static void event_button(SDL_Event *event);
  static void event_tab(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static SDL_FPoint load_wh();

  static inline std::optional<SDL_FPoint> drag;
  static inline uint8_t active_tab;
  static inline std::flat_set<int> disable_fold;
  static inline std::array<int, 2> pages;

public:
  static inline SDL_FPoint pos;
  static inline bool detail;
  static inline std::u16string quest; // quest

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};