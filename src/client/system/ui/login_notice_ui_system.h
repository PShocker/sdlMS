#pragma once

#include "src/client/system/ui/component/button.h"
#include "src/client/system_instance/login_notice_system_instance.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <vector>

namespace sdlms {

class LoginNoticeUiSystem {
 private:
  static SDL_FPoint LoadPos();
  static void RenderBackgrnd();
  static void RenderButton();
  static std::vector<Button>& GetComponent();

  static bool EventButton(SDL_Event* event);

  static inline std::vector<Button> components_;

 public:
  static void EventClose();

  static inline void (*fn_)();

  static inline LoginNoticeSystemInstance::NoticeType type_;
  static bool Render();
  static bool Run();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
