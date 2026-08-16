#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_npc.h"
#include "src/client/game/game_shop.h"
#include <array>
#include <cstdint>
#include <optional>
#include <string>
class shop_ui_system {
private:
  static void render_backgrnd();
  static void render_button();

  static void render_item(std::vector<game_shop_item> &items, int page,
                          SDL_FPoint p);

  static void render_items();
  static void render_tab();
  static void render_vscr();
  static void render_meso();

  static void render_npc();
  static void render_avatar();
  static void render_pkg_items();

  static void render_active_item();

  static void render_item_info(std::polymorphic<game_item> &item);

  static std::optional<int> event_item_click(SDL_Event *event, SDL_FPoint p);
  static bool event_item(SDL_Event *event);

  static bool event_vscr(SDL_Event *event);
  static void event_close();
  static bool event_button(SDL_Event *event);

  static void event_vscr_start(SDL_Event *event);
  static void event_vscr_end();
  static void event_vscr_move(SDL_Event *event);

  static inline std::array<bool, 2> vscr_motion;

  static bool event_tab(SDL_Event *event);

  static SDL_FPoint load_wh();

  static int add_item_slot(std::polymorphic<game_item> &item, int i);

public:
  static std::optional<int> add_item(std::polymorphic<game_item> &item);
  static void add_item_num(std::polymorphic<game_item> &item, int num);
  static void dec_item_num(std::polymorphic<game_item> &item, int num);

  static bool add_must_item(std::polymorphic<game_item> &item);

  static inline std::array<int, 2> pages;
  static std::vector<std::polymorphic<game_item> *> load_pkg_items();

  static inline std::vector<game_shop_item> must;

  static inline std::array<std::optional<int>, 2> active_item;
  static inline std::array<int, 2> active_tab;

  static inline std::optional<game_npc> npc;
  static inline const game_shop *shop;

  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};