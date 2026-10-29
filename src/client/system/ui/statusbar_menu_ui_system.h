#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <optional>
#include <vector>

namespace sdlms {

class StatusbarMenuUiSystem {
 private:
  static SDL_FRect LoadR();
  static void RenderBackgrnd();
  static void RenderUI();
  static UIComponents& GetComponent();
  static void RenderTitle();

  static void EventButtonCc();
  static void EventButtonOption();
  static void EventButtonQuit();

  static void EventButtonItem();
  static void EventButtonEquip();
  static void EventButtonStat();
  static void EventButtonSkill();
  static void EventButtonParty();
  static void EventButtonQuest();
  static void EventButtonCraft();

  static bool EventButton(SDL_Event* event);

  static inline int alpha_;

  static inline UIComponents components_;

 public:
  enum class MenuEnums {
    kMenu,
    kShortcut,
  };
  static inline std::optional<MenuEnums> menu_type_;

  static bool CursorIn();

  static void Toggle();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
