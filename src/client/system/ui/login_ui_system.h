#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "text_input_ui_system.h"
#include "uv.h"
#include <cstdint>
#include <string>
class LoginUiSystem {
private:
  static bool LoginAnimate();

  static SDL_FPoint LoadPos();
  static void RenderBackgrnd();
  static void RenderButton();
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

  static bool EventButton(SDL_Event *Event);

public:
  static inline uv_pipe_t g_pipe_;
  static inline TextInput username_;
  static bool CameraAnimate(float x, float y);

  static bool Render();
  static bool Event(SDL_Event *Event);
};