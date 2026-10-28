#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <flat_map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class PackageUiSystem {
private:
  static void RenderScroll();
  static void RenderBackgrnd();
  static void RenderTab();
  static void RenderItems();
  static bool RenderItemsInfo();
  static void RenderButton();
  static void RenderMeso();

  static void EventClose();
  static void EventButtonCoin();
  static void EventButtonGather();
  static void EventButtonSort();

  static void EventMotion(SDL_Event *Event);

  static bool EventClickItem(SDL_Event *Event);

  static bool EventVscr(SDL_Event *Event);
  static bool EventButton(SDL_Event *Event);
  static void EventTab(SDL_Event *Event);
  static void EventTop();
  static void EventDragStart(SDL_Event *Event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event *Event);

  static void EventVscrStart(SDL_Event *Event);
  static void EventVscrEnd();
  static void EventVscrMove(SDL_Event *Event);

  static void EventWheel(SDL_Event *Event);

  static void EventMotionBtAudio(SDL_Event *Event);

  static SDL_FPoint LoadWh();

  static inline bool vscr_motion;

  static inline uint8_t page;

  static inline std::optional<SDL_FPoint> drag;

public:
  static void RenderNumber(uint32_t num, int x, int y);

  static inline SDL_FPoint pos;
  static inline std::optional<uint32_t> mouse_index;
  static inline uint8_t active_tab;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event *Event);

  static bool CursorIn();

  static void Toggle();

  static void CalMouseIndex();

  struct NewItem {
    ItemEnum type;
    uint16_t index;
  };
  static inline std::optional<NewItem> new_itm;
};