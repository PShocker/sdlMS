#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_PACKAGE_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_PACKAGE_UI_SYSTEM_H_

#include "src/client/game/game_item.h"

#include <SDL3/SDL_events.h>

#include <cstdint>
#include <flat_map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

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

  static void EventMotion(SDL_Event* event);

  static bool EventClickItem(SDL_Event* event);

  static bool EventVscr(SDL_Event* event);
  static bool EventButton(SDL_Event* event);
  static void EventTab(SDL_Event* event);
  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static void EventVscrStart(SDL_Event* event);
  static void EventVscrEnd();
  static void EventVscrMove(SDL_Event* event);

  static void EventWheel(SDL_Event* event);

  static void EventMotionBtAudio(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static inline bool vscr_motion_;

  static inline uint8_t page_;

  static inline std::optional<SDL_FPoint> drag_;

 public:
  static void RenderNumber(uint32_t num, int x, int y);

  static inline SDL_FPoint pos_;
  static inline std::optional<uint32_t> mouse_index_;
  static inline uint8_t active_tab_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();

  static void CalMouseIndex();

  struct NewItem {
    ItemEnum type;
    uint16_t index;
  };
  static inline std::optional<NewItem> new_itm_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_PACKAGE_UI_SYSTEM_H_
