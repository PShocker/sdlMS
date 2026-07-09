#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_npc.h"
#include "src/client/game/game_shop.h"
#include <array>
#include <optional>
#include <string>
class shop_ui_system {
private:
  static void render_backgrnd();
  static void render_button();
  static void render_items();
  static void render_tab();
  static void render_vscr();

  static void render_npc();
  static void render_self();

  static void render_active_item();

  static void render_item_info(game_item& item);

  static bool event_item(SDL_Event *event);

  static void event_close();
  static bool event_button(SDL_Event *event);

  static void event_tab(SDL_Event *event);

  static SDL_FPoint load_wh();

  static bool event_open(SDL_Event *event);

  static inline std::array<int, 2> pages;
  static inline std::array<std::optional<int>, 2> active_item;

public:
  static inline std::array<int, 2> active_tab;

  static inline std::optional<game_npc> npc;
  static inline const game_shop* shop;

  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};