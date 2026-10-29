#pragma once

#include "src/client/system/ui/component/ui_component.h"
#include "src/client/game/game_chat.h"
#include "src/client/game/game_popup_tip.h"
#include "src/client/system/ui/component/text_input.h"
#include "src/common/flatbuffers/server.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

class StatusbarUiSystem : public UiComponentHost<StatusbarUiSystem> {
 private:
  static void RenderHmAlert();
  static void RenderGaugeText();
  static void RenderBackgrnd();
  static void RenderCharacterStat();
  static void RenderChat();
  static void RenderBackChat();

  static SDL_FPoint LoadWh();

  static void EventChatSend();
  static std::u16string LoadChatType();

  static void EventMotionBtAudio(SDL_Event* event);

  static bool EventUI(SDL_Event* event);
  // button_func
  static void EventButtonCashshop();
  static void EventButtonMenu();
  static void EventButtonShortcut();
  static void EventButtonMailbox();
  static void EventButtonEquip();
  static void EventButtonInven();
  static void EventButtonStat();
  static void EventButtonSkill();
  static void EventButtonKeybind();
  static void EventButtonQuickslot();
  static void EventButtonChatlog();


 public:
  static UIComponents& GetComponent();
  static inline std::optional<GameChatEnum> chat_type_;
  static inline TextInput chat_;

  static void Reset();
  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();
};

}  // namespace sdlms
