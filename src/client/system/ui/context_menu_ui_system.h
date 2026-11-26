#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

class ContextMenuUiSystem : public UiComponentHost<ContextMenuUiSystem> {
 private:
  static SDL_FPoint LoadWh();
  static void RenderBackgrnd();

  static void EventClose();
  static void EventButtonInfo();
  static void EventButtonSave();
  static void EventButtonWhisper();
  static void EventButtonParty();
  static void EventButtonTrade();
  static void EventButtonFriend();
  // MapleChat 聊天邀请（BtMapleChat 图形即 INVITE TO CHAT）。
  static void EventButtonMapleChat();
  // 摆摊入口（无对应按钮贴图，自绘 SHOP 行）。
  static void EventButtonChat();

  static bool EventUI(SDL_Event* event);


 public:
  static UIComponents& GetComponent();
  static inline uint64_t client_id_;
  static inline std::u16string client_name_;

  static inline SDL_FPoint pos_;
  static bool Render();

  static bool CursorIn();

  static void Open();
  static void Close();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
