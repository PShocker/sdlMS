#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>

namespace sdlms {

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

  static bool EventButton(SDL_Event* event);

 public:
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
