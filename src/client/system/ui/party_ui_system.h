#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_PARTY_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_PARTY_UI_SYSTEM_H_

#include <SDL3/SDL_events.h>
#include <optional>
namespace sdlms {

class PartyUiSystem {
private:
  static void RenderBackgrnd(float x, float y, float w, float h);
  static void RenderPartyHp();

  static void EventTop();
  static void EventDragStart(SDL_Event *Event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event *Event);

  static SDL_FPoint LoadWh();

  static inline std::optional<SDL_FPoint> drag_;

public:
  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event *Event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_PARTY_UI_SYSTEM_H_
