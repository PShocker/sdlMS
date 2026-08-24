#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

class equip_ui_system {
public:
  enum equip_mouse_index {
    cap,
    earcc,
    clothes,
    pants,
    shoes,
    gloves,
    cape,
    shield,
    weapon,
    ring0,
    ring1,
    ring2,
    ring3,
  };

private:
  static std::optional<equip_mouse_index> load_mouse_index();

  static void render_backgrnd();
  static void render_backgrnd2();
  static void render_tab();
  static void render_equip();
  static void render_deco();
  static void render_disable_texture(SDL_FPoint slot);
  static void render_equip_texture(game_equip_item &equip, SDL_FPoint slot);
  static void render_deco_texture(game_deco_item &deco, SDL_FPoint slot);
  static bool render_info();
  static void render_button();

  static SDL_FPoint load_wh();

  static void event_close();

  static bool event_click_equip(SDL_Event *event);

  static void event_motion(SDL_Event *event);
  static void event_tab(SDL_Event *event);
  static bool event_button(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static inline std::optional<SDL_FPoint> drag;
  static inline uint8_t active_tab;
public:
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
  static void toggle();
};