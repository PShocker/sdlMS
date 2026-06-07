#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_equip.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <flat_map>
#include <optional>

class package_ui_system {
private:
  static std::optional<uint32_t> load_mouse_index();

  static void render_scroll();
  static void render_backgrnd();
  static void render_tab();
  static void render_items();
  static void render_items_info();
  static void render_button();

  static void event_close();

  static bool event_click_item(SDL_Event *event);

  static bool event_button(SDL_Event *event);
  static void event_tab(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static SDL_FPoint load_wh();

  static bool run();

  static inline uint8_t page;

  static inline std::optional<SDL_FPoint> drag;
public:
  static inline SDL_FPoint pos;

  static inline uint8_t active_tab;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};