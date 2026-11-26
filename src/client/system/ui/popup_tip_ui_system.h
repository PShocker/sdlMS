#pragma once

#include "src/client/game/game_popup_tip.h"

#include <SDL3/SDL_events.h>

namespace sdlms {

class PopupTipUiSystem {
 private:
  static void RenderBackgrnd(GamePopupTip& t);
  static void RenderText(GamePopupTip& t);
  static void RenderButton(GamePopupTip& t);

  // 组队邀请确认框：accept 1=接受 0=拒绝。
  static void EventButtonParty(GamePopupTip& t, uint8_t accept);
  static void EventButtonTrade(GamePopupTip& t);
  // 五子棋请求（悔棋/求和）确认框：accept 1=同意 0=拒绝。
  static void EventButtonOmok(GamePopupTip& t, uint8_t accept);
  // MapleChat 聊天邀请确认框：accept 1=进房 0=拒绝并回执邀请人。
  static void EventButtonMapleChat(GamePopupTip& t, uint8_t accept);
  static void EventButtonOk(GamePopupTip& t);
  static void EventButtonCancel(GamePopupTip& t);

  static bool EventUI(SDL_Event* event);

  static void Destroy();

 public:
  static inline SDL_FPoint pos_ = {429, -32};

  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
