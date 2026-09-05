#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_item.h"
#include "text_input_ui_system.h"
#include <any>
class notice_ui_system {
private:
  static void render_backgrnd();
  static void render_button();
  static void render_text();
  static void render_input();

  static void event_close();

  static void event_button_shopbuy();
  static void event_button_shopbuy_must();
  static void event_button_shopbuy_sell();

  static void event_button_throw_mul();
  static void event_button_throw_close();

  static void event_button_worldmap_teleport();

  static void event_button_ap_inc();

  static void event_button_equip_no_ap();

  static bool event_button(SDL_Event *event);
  static void event_input(SDL_Event *event);

  static SDL_FPoint load_wh();

public:
  enum class notice_enum {
    shopbuy,
    shopbuy_sell,
    shopbuy_sell_mul,
    shopbuy_sell_no_num,
    shopbuy_mul,
    shopbuy_no_meso,

    no_equip_space,
    no_consume_space,
    no_install_space,
    no_etc_space,
    no_cash_space,
    no_deco_space,

    no_storage_space,

    equip_no_ability,
    equip_no_space,

    ap_inc,
    worldmap_disable,
    worldmap_teleport,

    throw_mul,

    trade_block,
  };

  static inline notice_enum type;
  static inline SDL_FPoint pos;
  static inline text_input text;
  static inline std::any data;

  static bool render();

  static void open();
  static void open_no_space(item_enum type);
  static void close();

  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};