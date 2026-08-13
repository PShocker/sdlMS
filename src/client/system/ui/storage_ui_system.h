#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_item.h"
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>


class storage_ui_system {
private:
  static bool add_item(std::polymorphic<game_item> &item);

  static std::optional<uint32_t> load_mouse_index();
  static SDL_FPoint load_wh();

  static void render_backgrnd();
  static void render_button();
  static void render_tab();

  static void render_items();
  static void render_item_info();
  static void render_vscr();

  static void event_tab();
  static void event_button_get();
  static void event_button_put();
  static void event_button_sort();
  static void event_button_exit();
  static bool event_button(SDL_Event *event);
  static void event_close();

  static inline std::array<std::optional<uint16_t>, 2> active_item;
  static inline int active_tab;
  static inline std::array<uint16_t, 2> page;

public:
  static inline std::u16string npc_id;
  static inline SDL_FPoint pos;

  static bool cursor_in();

  static void toggle();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};