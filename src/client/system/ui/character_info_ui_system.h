#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_CHARACTER_INFO_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_CHARACTER_INFO_UI_SYSTEM_H_

#include "src/client/game/game_character.h"
#include "src/client/game/game_item.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace sdlms {

class CharacterInfoUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderButton();
  static void RenderCharacter();
  static void RenderText();
  static void RenderItems();
  static bool RenderItemsInfo();
  static void RenderScroll();

  static SDL_FPoint LoadWh();
  static void EventTop();
  static void EventDragStart(SDL_Event* Event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* Event);

  static bool EventVscr(SDL_Event* Event);

  static void EventButtonParty();
  static void EventButtonTrade();
  static void EventButtonToggleitem();
  static void EventButtonToggleitemHide();
  static void EventButtonCitizenship();
  static void EventButtonUpFame();
  static void EventButtonDowmFame();
  static void EventButtonShowPet();
  static void EventClose();

  static void EventMotionBtAudio(SDL_Event* Event);
  static void EventMotion(SDL_Event* Event);

  static bool EventButton(SDL_Event* Event);

  static void EventWheel(SDL_Event* Event);

  static void EventVscrStart(SDL_Event* Event);
  static void EventVscrEnd();
  static void EventVscrMove(SDL_Event* Event);

  static inline bool vscr_motion_;

  static bool Run();

  static inline std::optional<SDL_FPoint> drag_;

  static inline bool item_;
  static inline uint8_t item_page_;
  static inline std::optional<std::variant<GameEquipItem, GameDecoItem>>
      item_info_;

  static inline bool pet_;

 public:
  static inline GameCharacter character_;

  static inline SDL_FPoint pos_;

  static bool CursorIn();

  static void Toggle();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* Event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_CHARACTER_INFO_UI_SYSTEM_H_
