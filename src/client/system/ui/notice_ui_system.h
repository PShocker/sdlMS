#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_NOTICE_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_NOTICE_UI_SYSTEM_H_

#include "src/client/game/game_item.h"
#include "src/client/system/ui/text_input_ui_system.h"

#include <SDL3/SDL_events.h>

#include <any>

namespace sdlms {

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

  static bool EventButton(SDL_Event* event);
  static void EventInput(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static void EventMotionBtAudio(SDL_Event* event);

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

  static inline NoticeEnum type_;
  static inline SDL_FPoint pos_;
  static inline TextInput text_;
  static inline std::any data_;

  static bool Render();

  static void Open();
  static void OpenNoSpace(ItemEnum type);
  static void Close();

  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_NOTICE_UI_SYSTEM_H_
