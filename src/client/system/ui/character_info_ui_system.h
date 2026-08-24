#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>
class character_info_ui_system {
private:
  static void render_backgrnd();
  static void render_button();
  static void render_character();
  static void render_text();
  static void render_items();
  static void render_items_info();
  static void render_scroll();

  static SDL_FPoint load_wh();
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static bool event_vscr(SDL_Event *event);

  static void event_button_party();
  static void event_button_trade();
  static void event_button_toggleitem();
  static void event_button_toggleitem_hide();
  static void event_button_wishlist();
  static void event_button_up_fame();
  static void event_button_dowm_fame();
  static void event_button_show_pet();
  static void event_close();

  static bool event_button(SDL_Event *event);

  static void event_vscr_start(SDL_Event *event);
  static void event_vscr_end();
  static void event_vscr_move(SDL_Event *event);

  static inline bool vscr_motion;

  static bool run();

  static inline std::optional<SDL_FPoint> drag;

  static inline bool item;
  static inline uint8_t item_page;
  static inline std::optional<std::variant<game_equip_item, game_deco_item>>
      item_info;

  static inline bool pet;

public:
  static inline game_character character;

  static inline SDL_FPoint pos;

  static bool cursor_in();

  static void toggle();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};