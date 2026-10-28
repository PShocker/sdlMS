#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <flat_set>
#include <optional>
#include <string>
class QuestAlarmUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderQuests();
  static void RenderButton();
  static void RenderTitle();

  static SDL_FPoint LoadWh();

  static void EventButtonAuto();
  static void EventButtonMax();
  static void EventButtonMin();
  static void EventButtonClose();

  static void EventQuestClose();

  static bool EventButton(SDL_Event *Event);

  static void EventTop();
  static void EventDragStart(SDL_Event *Event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event *Event);

  static inline std::optional<SDL_FPoint> drag;

public:
  static void AcceptQuest(const std::u16string &id);
  static void CompleteQuest(const std::u16string &id);
  static inline std::flat_set<std::u16string> quests;

  static inline bool max = true;

  static inline SDL_FPoint pos;

  static void EventAutoQuests();

  static void Open();
  static void Close();

  static bool CursorIn();

  static bool Render();
  static bool Event(SDL_Event *Event);
};