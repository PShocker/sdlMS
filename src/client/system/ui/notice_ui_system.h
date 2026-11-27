#pragma once

#include "src/client/system/ui/component/ui_component.h"
#include "src/client/game/game_item.h"
#include "src/client/system/ui/component/text_input.h"

#include <SDL3/SDL_events.h>

#include <any>
#include <functional>
#include <string>
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

  static void EventButtonShopRecharge();

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
    kCannotThrow,
    kCannotChangeEquip,

    kQuestGiveup,

    kShopRecharge,
    kCraftState,

    kMinigameRoomFull,
    // 摆摊：上架定价（数字输入，确认走 on_confirm_）。
    kShopPrice,
    // 摆摊：通用文本提示（文本放 notice_text_）。
    kShopNotice,
    // 仓库：数字输入（提示文本放 notice_text_，默认值放 number_value_）。
    kStorageCount,
  };

  static inline NoticeEnum type_;
  static inline SDL_FPoint pos_;
  static inline TextInput text_;
  static inline std::any data_;
  static inline std::function<void()> on_confirm_;
  // kShopNotice 的提示文本。
  static inline std::u16string notice_text_;
  // kStorageCount 输入框的初始值。
  static inline uint64_t number_value_ = 0;

  static bool Render();

  static void Open();
  // 读取当前输入框里的数字（无输入时返回 0）。
  static uint64_t LoadInputNumber();
  static void OpenNoSpace(ItemEnum type);
  static void Close();

  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
