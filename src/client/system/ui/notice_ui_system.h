#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_item.h"
#include "text_input_ui_system.h"
#include <any>
class NoticeUiSystem {
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

  static void event_motion_bt_audio(SDL_Event *event);

public:
  enum class NoticeEnum {
    kShopbuy,
    kShopbuySell,
    kShopbuySellMul,
    kShopbuySellNoNum,
    kShopbuyMul,
    kShopbuyNoMeso,

    kNoEquipSpace,
    kNoConsumeSpace,
    kNoInstallSpace,
    kNoEtcSpace,
    kNoCashSpace,
    kNoDecoSpace,

    kNoStorageSpace,

    kEquipNoAbility,
    kEquipNoSpace,

    kApInc,
    kWorldmapDisable,
    kWorldmapTeleport,

    kThrowMul,

    kTradeBlock,
  };

  static inline NoticeEnum type;
  static inline SDL_FPoint pos;
  static inline TextInput text;
  static inline std::any data;

  static bool render();

  static void open();
  static void open_no_space(ItemEnum type);
  static void close();

  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};