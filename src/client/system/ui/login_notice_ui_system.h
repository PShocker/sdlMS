#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/system_instance/login_notice_system_instance.h"
namespace sdlms {

class LoginNoticeUiSystem {
private:
  static SDL_FPoint LoadPos();
  static void RenderBackgrnd();
  static void RenderButton();

  static bool EventButton(SDL_Event *Event);

public:
  static void EventClose();

  static inline void (*fn)();

  static inline LoginNoticeSystemInstance::NoticeType type_;
  static bool Render();
  static bool Run();
  static bool Event(SDL_Event *Event);
};

}  // namespace sdlms
