#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_shop.h"
#include <cstdint>
#include <optional>
#include <string>
class shop_ui_system {
private:
  static void render_backgrnd();
  static void render_button();
  static void render_items();
  static void render_tab();

  static bool event_button(SDL_Event *event);

  static void event_tab(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static SDL_FPoint load_wh();

  static inline std::optional<SDL_FPoint> drag;

  static bool event_open(SDL_Event *event);

  static inline int32_t page;
public:
  static inline uint8_t active_tab;

  static inline std::optional<std::u16string> npc_id;
  static inline std::optional<game_shop> shop;
  
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};