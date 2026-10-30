#pragma once

#include "src/client/system/ui/component/ui_component.h"
#include "src/client/game/game_item.h"
#include "src/client/system/ui/component/text_input.h"

#include <SDL3/SDL_events.h>

#include <any>
#include <functional>
#include <vector>

namespace sdlms {

class NoticeUiSystem : public UiComponentHost<NoticeUiSystem> {
 private:
  static void RenderBackgrnd();
  static void RenderText();
  static void RenderInput();

  static void EventClose();

  static void EventButtonShopbuy();
  static void EventButtonShopbuyMust();
  static void EventButtonShopbuySell();

  static void EventButtonThrowMul();
  static void EventButtonThrowClose();

  static void EventButtonQuestGiveup();

  static void EventButtonWorldmapTeleport();

  static void EventButtonApInc();

  static void EventButtonEquipNoAp();

  static bool EventUI(SDL_Event* event);
  static void EventInput(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static void EventMotionBtAudio(SDL_Event* event);


 public:
  static UIComponents& GetComponent();
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

    kQuestGiveup,

    kTradeBlock,
  };

  static inline NoticeEnum type_;
  static inline SDL_FPoint pos_;
  static inline TextInput text_;
  static inline std::any data_;
  static inline std::function<void()> on_confirm_;

  static bool Render();

  static void Open();
  static void OpenNoSpace(ItemEnum type);
  static void Close();

  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
