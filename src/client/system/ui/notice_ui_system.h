#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_item.h"
#include "text_input_ui_system.h"
#include <any>
class NoticeUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderButton();
  static void RenderText();
  static void RenderInput();

  static void EventClose();

  static void EventButtonShopbuy();
  static void EventButtonShopbuyMust();
  static void EventButtonShopbuySell();

  static void EventButtonThrowMul();
  static void EventButtonThrowClose();

  static void EventButtonWorldmapTeleport();

  static void EventButtonApInc();

  static void EventButtonEquipNoAp();

  static bool EventButton(SDL_Event *Event);
  static void EventInput(SDL_Event *Event);

  static SDL_FPoint LoadWh();

  static void EventMotionBtAudio(SDL_Event *Event);

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

  static bool Render();

  static void Open();
  static void OpenNoSpace(ItemEnum type);
  static void Close();

  static bool Event(SDL_Event *Event);

  static bool CursorIn();

  static void Toggle();
};