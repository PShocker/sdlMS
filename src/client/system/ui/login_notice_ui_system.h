#pragma once

#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system_instance/login_notice_system_instance.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <vector>

namespace sdlms {

class LoginNoticeUiSystem {
 private:
  static SDL_FPoint LoadPos();
  static void RenderBackgrnd();
  static void RenderUI();
  static UIComponents& GetComponent();

  static bool EventUI(SDL_Event* event);

  static inline UIComponents components_;

 public:
  static void EventClose();

  static inline void (*fn_)();

  static inline LoginNoticeSystemInstance::NoticeType type_;
  static bool Render();
  static bool Run();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
