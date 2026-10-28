#ifndef SDLMS_SRC_CLIENT_SYSTEM_INPUT_INPUT_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_INPUT_INPUT_SYSTEM_H_

#include "src/client/game/game_input.h"

namespace sdlms {

class InputSystem {
 public:
  static void HandleUiInput(GameInput& g_input);
  static void HandleItemInput(GameInput& g_input, bool press);
  static void HandleActionInput(GameInput& g_input, bool press);
  static void HandleSkillInput(GameInput& g_input, bool press);
  static void HandleFaceInput(GameInput& g_input, bool press);
  static void Reset();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_INPUT_INPUT_SYSTEM_H_
