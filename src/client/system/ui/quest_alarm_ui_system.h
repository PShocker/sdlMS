#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_QUEST_ALARM_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_QUEST_ALARM_UI_SYSTEM_H_

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <flat_set>
#include <optional>
#include <string>
namespace sdlms {

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

  static inline std::optional<SDL_FPoint> drag_;

public:
  static void AcceptQuest(const std::u16string &id);
  static void CompleteQuest(const std::u16string &id);
  static inline std::flat_set<std::u16string> quests_;

  static inline bool max_ = true;

  static inline SDL_FPoint pos_;

  static void EventAutoQuests();

  static void Open();
  static void Close();

  static bool CursorIn();

  static bool Render();
  static bool Event(SDL_Event *Event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_QUEST_ALARM_UI_SYSTEM_H_
