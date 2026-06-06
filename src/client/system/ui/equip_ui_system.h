#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_equip.h"
#include <cstdint>
#include <optional>
#include <string>
class equip_ui_system {
private:
  static void render_backgrnd();
  static void render_backgrnd2();
  static void render_tab();
  static void render_equip();
  static void render_equip_texture(game_equip &equip, SDL_FPoint slot);
  static void render_equip_info();
  static void render_button();

  static SDL_FPoint load_wh();

  static void event_close();

  static bool event_button(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static inline std::optional<SDL_FPoint> drag;
  static inline uint8_t active_tab;
  static inline std::optional<game_equip> equip_info;

public:
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
  static void toggle();
};