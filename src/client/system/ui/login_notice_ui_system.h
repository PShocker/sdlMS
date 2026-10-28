#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_LOGIN_NOTICE_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_LOGIN_NOTICE_UI_SYSTEM_H_

#include "src/client/system_instance/login_notice_system_instance.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

namespace sdlms {

class LoginNoticeUiSystem {
 private:
  static SDL_FPoint LoadPos();
  static void RenderBackgrnd();
  static void RenderButton();

  static bool EventButton(SDL_Event* event);

 public:
  static void EventClose();

  static inline void (*fn)();

  static inline LoginNoticeSystemInstance::NoticeType type_;
  static bool Render();
  static bool Run();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_LOGIN_NOTICE_UI_SYSTEM_H_
