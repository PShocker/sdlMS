#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <flat_map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class package_ui_system {
private:
  static void render_scroll();
  static void render_backgrnd();
  static void render_tab();
  static void render_items();
  static bool render_items_info();
  static void render_button();
  static void render_meso();

  static void event_close();
  static void event_motion(SDL_Event *event);

  static bool event_click_item(SDL_Event *event);

  static bool event_vscr(SDL_Event *event);
  static bool event_button(SDL_Event *event);
  static void event_tab(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static void event_vscr_start(SDL_Event *event);
  static void event_vscr_end();
  static void event_vscr_move(SDL_Event *event);

  static SDL_FPoint load_wh();

  static inline bool vscr_motion;

  static inline uint8_t page;

  static inline std::optional<SDL_FPoint> drag;

public:
  static std::optional<int> add_item(std::polymorphic<game_item> &item);
  static void add_item_num(std::polymorphic<game_item> &item, int num);
  static void dec_item_num(std::polymorphic<game_item> &item, int num);

  static void render_number(uint32_t num, int x, int y);

  static std::vector<uint32_t> load_blank_index(uint32_t tab);
  static std::vector<uint32_t> load_b_index(std::polymorphic<game_item> &item);

  static std::polymorphic<game_item> *load_active_ball();
  static uint32_t load_full_item_num(const std::u16string &id);
  static std::polymorphic<game_item> *load_f_item(const std::u16string &id);

  static inline SDL_FPoint pos;

  static inline uint8_t active_tab;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();

  static std::optional<uint32_t> load_mouse_index();

  struct new_item {
    item_enum type;
    uint16_t index;
  };
  static inline std::optional<new_item> new_itm;
};