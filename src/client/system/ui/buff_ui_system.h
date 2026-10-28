#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_BUFF_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_BUFF_UI_SYSTEM_H_

#include "src/client/game/game_item_buff.h"
#include "src/client/game/game_skill.h"

#include <SDL3/SDL_events.h>

#include <cstdint>

namespace sdlms {

class BuffUiSystem {
 private:
  static void RenderUi();
  static void RenderUi(GameSkill& sk, float x, float y);
  static void RenderUi(GameItemBuff& gb, float x, float y);
  static void EventMotion(SDL_Event* Event);

 public:
  static bool RenderUiInfo();
  static bool Render();
  static bool Event(SDL_Event* Event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_BUFF_UI_SYSTEM_H_
