#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_QUEST_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_QUEST_UI_SYSTEM_H_

#include "src/client/game/game_quest.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <array>
#include <flat_set>
#include <optional>
#include <string>

namespace sdlms {

class QuestUiSystem {
 private:
  static int LoadVscrNum0();
  static int LoadVscrNum1();
  static int LoadDetailSummaryBackgrndH();
  static std::u16string LoadDetailStr();

  static void RenderBackgrnd();
  static void RenderButton();
  static void RenderQuests();
  static void RenderQuestDetail();
  static void RenderTab();
  static void RenderAreaName(int i, int y);
  static void RenderQuest(GameQuest& q, int y);
  static void RenderVscr();
  static void RenderDetailVscr();
  static void RenderDetailAvaliable();
  static void RenderDetailProgress();
  static void RenderDetailSummary();
  static void RenderDetailSummaryBackgrnd();
  static bool RenderInfo();

  static void EventFold();
  static void EventQuest();

  static void EventButtonClose();
  static void EventButtonDetail();

  static bool EventCheckbox();
  static bool EventVscr(SDL_Event* event);
  static bool EventButton(SDL_Event* event);
  static void EventTab(SDL_Event* event);
  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static void EventWheel(SDL_Event* event);

  static void EventVscrStart(SDL_Event* event);
  static void EventVscrEnd();
  static void EventVscrMove(SDL_Event* event);

  static void EventMotionBtAudio(SDL_Event* event);
  static void EventMotion(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static inline std::optional<SDL_FPoint> drag_;

  static inline std::array<bool, 2> vscr_motion_;

 public:
  static void AcceptQuest(const std::u16string& id);
  static void CompleteQuest(const std::u16string& id);
  static inline SDL_FPoint pos_;
  static inline bool detail_;
  static inline uint8_t active_tab_;
  static inline std::flat_set<int> disable_fold_;
  static inline std::array<int, 2> pages_;
  static inline std::u16string detail_quest_;  // quest

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_QUEST_UI_SYSTEM_H_
