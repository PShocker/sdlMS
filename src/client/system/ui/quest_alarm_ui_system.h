#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <flat_set>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

class QuestAlarmUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderQuests();
  static void RenderUI();
  static UIComponents& GetComponent();
  static void RenderTitle();

  static SDL_FPoint LoadWh();

  static void EventButtonAuto();
  static void EventButtonMax();
  static void EventButtonMin();
  static void EventButtonClose();

  static void EventQuestClose();

  static bool EventButton(SDL_Event* event);

  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static inline std::optional<SDL_FPoint> drag_;

  static inline UIComponents components_;

 public:
  static void AcceptQuest(const std::u16string& id);
  static void CompleteQuest(const std::u16string& id);
  static inline std::flat_set<std::u16string> quests_;

  static inline bool max_ = true;

  static inline SDL_FPoint pos_;

  static void EventAutoQuests();

  static void Open();
  static void Close();

  static bool CursorIn();

  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
