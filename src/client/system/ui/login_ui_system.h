#pragma once

#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system/ui/component/text_input.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <uv.h>

#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

class LoginUiSystem {
 private:
  static bool LoginAnimate();

  static SDL_FPoint LoadPos();
  static void RenderBackgrnd();
  static void RenderUI();
  static UIComponents& GetComponent();
  static void RenderEffect();
  static void RenderBanner();
  static void RenderUsername();

  static void EventButtonLogin();
  static void EventButtonLoginSave();
  static void EventButtonFindId();
  static void EventButtonFindPw();
  static void EventButtonRegister();
  static void EventButtonHomepage();
  static void EventButtonQuit();
  static void EventButtonBack();

  static bool EventButton(SDL_Event* event);

  static inline UIComponents components_;

 public:
  static inline uv_pipe_t g_pipe_;
  static inline TextInput username_;
  static bool CameraAnimate(float x, float y);

  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
