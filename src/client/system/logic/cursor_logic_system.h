#pragma once

#include "src/client/game/game_npc.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <optional>
#include <string>

namespace sdlms {

class CursorLogicSystem {
 private:
  static std::optional<GameNpc> CursorNpc();

  static bool RunPackageMotion();
  static bool RunVscrollMotion();
  static bool RunAnimate();

  static void RunCursorUi();
  static void RunCursorAction(const std::u16string& action);

  static bool RunDefault();

  static bool EventCursorHand(SDL_Event* event);
  static bool EventCharacter(SDL_Event* event);
  static bool EventNpc(SDL_Event* event);

 public:
  static bool Run();

  static bool EventClickNpc(GameNpc npc);
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
