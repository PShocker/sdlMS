
#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class ContextMenuUiSystem {
private:
  static SDL_FPoint LoadWh();
  static void RenderBackgrnd();
  static void RenderButton();

  static void EventClose();
  static void EventButtonInfo();
  static void EventButtonSave();
  static void EventButtonWhisper();
  static void EventButtonParty();
  static void EventButtonTrade();
  static void EventButtonFriend();
  static void EventButtonChat();

  static bool EventButton(SDL_Event *Event);

public:
  static inline uint64_t client_id;
  static inline std::u16string client_name;

  static inline SDL_FPoint pos;
  static bool Render();

  static bool CursorIn();

  static void Open();
  static void Close();
  static bool Event(SDL_Event *Event);
};